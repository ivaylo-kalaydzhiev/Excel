#include "CellEvaluator.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>

// Helper to convert LiteralValue to string for output
std::string literalValueToString(const LiteralValue& lv) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "TRUE" : "FALSE";
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        }
        return "#VALUE!"; // Should not happen with current LiteralValue types
        }, lv.value);
}

// Helper to extract a double from LiteralValue, returns NaN if not numeric
double getNumericValue(const LiteralValue& lv) {
    return std::visit([](auto&& arg) -> double {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return static_cast<double>(arg);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return static_cast<double>(arg); // true=1.0, false=0.0
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            // Attempt to convert string to double
            try {
                size_t pos;
                double val = std::stod(arg, &pos);
                if (pos == arg.length()) { // Entire string converted
                    return val;
                }
            }
            catch (...) {
                // Conversion failed or partial conversion
            }
            return std::numeric_limits<double>::quiet_NaN(); // Not a number
        }
        return std::numeric_limits<double>::quiet_NaN(); // Fallback for unexpected types
        }, lv.value);
}

// Helper to extract a string from LiteralValue
std::string getStringValue(const LiteralValue& lv) {
    auto lambda = [&](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        }
        else {
            return literalValueToString(lv);
        }
        };
    return std::visit(lambda, lv.value);
}


CellEvaluator::CellEvaluator(const TableModel& model) : model(model) {
    // Constructor initializes the model reference
}

// Public evaluation interface
std::string CellEvaluator::evaluate(const CellValue& cellValue) {
    try {
        LiteralValue result = resolve(cellValue);
        return literalValueToString(result);
    }
    catch (const std::runtime_error& e) {
        return e.what(); // Return error message
    }
    catch (const std::invalid_argument& e) {
        return e.what(); // Return error message
    }
    catch (...) {
        return "#ERROR!"; // Generic error
    }
}

// Resolves a CellValue into a LiteralValue
LiteralValue CellEvaluator::resolve(const CellValue& value) {
    if (value.isLiteral()) {
        // If it's a raw string, double, or bool
        return std::visit([](auto&& arg) -> LiteralValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return LiteralValue{ arg };
            }
            else if constexpr (std::is_same_v<T, double>) {
                // Convert double to int if it's a whole number, otherwise keep as string for precision
                if (arg == static_cast<int>(arg)) {
                    return LiteralValue{ static_cast<int>(arg) };
                }
                else {
                    return LiteralValue{ std::to_string(arg) }; // Store as string to preserve precision
                }
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return LiteralValue{ arg };
            }
            return LiteralValue{ "#VALUE!" }; // Should not happen
            }, value.value);
    }
    else if (value.isReference()) {
        // If it's a cell reference, resolve the target cell's value
        const CellAddress& targetAddress = std::get<CellAddress>(value.value);
        const CellValue* targetCellValue = model.getCellValue(targetAddress);
        if (!targetCellValue) {
            return LiteralValue{ "#REF!" }; // Reference to an empty or non-existent cell
        }
        // Recursive call to resolve the referenced cell's value
        return resolve(*targetCellValue);
    }
    else if (value.isFormula()) {
        // If it's a formula, evaluate it
        const FormulaValue& formula = std::get<FormulaValue>(value.value);
        return evaluateFormula(formula);
    }
    return LiteralValue{ "#VALUE!" }; // Unknown CellValue type
}

// Evaluates a FormulaValue
LiteralValue CellEvaluator::evaluateFormula(const FormulaValue& formula) {
    try {
        switch (formula.type) {
        case FormulaType::SUM:
            return evalSUM(formula.parameters);
        case FormulaType::AVERAGE:
            return evalAVERAGE(formula.parameters);
        case FormulaType::MIN:
            return evalMIN(formula.parameters);
        case FormulaType::MAX:
            return evalMAX(formula.parameters);
        case FormulaType::CONCAT:
            return evalCONCAT(formula.parameters);
        case FormulaType::SUBSTR:
            return evalSUBSTR(formula.parameters);
        case FormulaType::LEN:
            return evalLEN(formula.parameters);
        case FormulaType::COUNT:
            return evalCOUNT(formula.parameters);
        default:
            return LiteralValue{ "#NAME?" }; // Unknown formula type
        }
    }
    catch (const std::runtime_error& e) {
        return LiteralValue{ std::string("#ERROR! ") + e.what() };
    }
    catch (const std::invalid_argument& e) {
        return LiteralValue{ std::string("#ERROR! ") + e.what() };
    }
    catch (...) {
        return LiteralValue{ "#ERROR!" };
    }
}

// Helper to check if any LiteralValue in a vector is an error literal
bool CellEvaluator::containsErrorLiteral(const std::vector<LiteralValue>& values) {
    for (const auto& lv : values) {
        if (std::holds_alternative<std::string>(lv.value)) {
            const std::string& s = std::get<std::string>(lv.value);
            if (s == "#VALUE!" || s == "#REF!" || s.rfind("#ERROR!", 0) == 0 || s == "#DIV/0!" || s == "#NAME?") {
                return true;
            }
        }
    }
    return false;
}

// Flattens a vector of FormulaParam into a vector of resolved LiteralValue
std::vector<LiteralValue> CellEvaluator::flattenArgs(const std::vector<FormulaParam>& args) {
    std::vector<LiteralValue> flattened;
    for (const auto& param : args) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, LiteralValue>) {
                flattened.push_back(arg);
            }
            else if constexpr (std::is_same_v<T, CellAddress>) {
                const CellValue* cellValue = model.getCellValue(arg);
                if (cellValue) {
                    flattened.push_back(resolve(*cellValue));
                }
                else {
                    // Per requirements, empty cell evaluates to 0 for numeric context, or empty string for others
                    // For now, push empty string, numeric functions will convert to NaN or 0 as needed
                    flattened.push_back(LiteralValue{ "" });
                }
            }
            else if constexpr (std::is_same_v<T, AddressRange>) {
                std::vector<LiteralValue> rangeValues = expandRange(arg);
                flattened.insert(flattened.end(), rangeValues.begin(), rangeValues.end());
            }
            }, param);
    }
    return flattened;
}

// Expands an AddressRange into a vector of resolved LiteralValue for each cell in the range
std::vector<LiteralValue> CellEvaluator::expandRange(const AddressRange& range) {
    std::vector<LiteralValue> expandedValues;

    // Determine the min/max row and column
    int startRow = range.start.row;
    int startCol = range.start.column;
    int endRow = range.end.row;
    int endCol = range.end.column;

    int minRow = std::min(startRow, endRow);
    int maxRow = std::max(startRow, endRow);
    int minCol = std::min(startCol, endCol);
    int maxCol = std::max(startCol, endCol);

    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            CellAddress currentAddress{ r, c };
            const CellValue* cellValue = model.getCellValue(currentAddress);
            if (cellValue) {
                expandedValues.push_back(resolve(*cellValue));
            }
            else {
                // If a cell in the range is empty, treat it as empty string.
                // Numeric functions will convert "" to NaN, which is then handled.
                expandedValues.push_back(LiteralValue{ "" }); // Represents an empty cell
            }
        }
    }
    return expandedValues;
}

// --- Formula Evaluation Functions ---

LiteralValue CellEvaluator::evalSUM(const std::vector<FormulaParam>& args) {
    std::vector<LiteralValue> values = flattenArgs(args);
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }
    if (values.empty()) { // Must have at least 1 parameter
        return LiteralValue{ "#VALUE!" };
    }

    double sum = 0.0;
    bool hasNumeric = false;

    for (const auto& lv : values) {
        double num = getNumericValue(lv);
        if (!std::isnan(num)) {
            sum += num;
            hasNumeric = true;
        }
    }

    if (!hasNumeric) { // If no numeric cells found
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ static_cast<int>(sum) }; // Convert to int if whole number
}

LiteralValue CellEvaluator::evalAVERAGE(const std::vector<FormulaParam>& args) {
    std::vector<LiteralValue> values = flattenArgs(args);
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }
    if (values.empty()) { // Must have at least 1 parameter
        return LiteralValue{ "#VALUE!" };
    }

    double sum = 0.0;
    int count = 0;

    for (const auto& lv : values) {
        double num = getNumericValue(lv);
        if (!std::isnan(num)) {
            sum += num;
            count++;
        }
    }

    if (count == 0) { // If no numeric values found
        return LiteralValue{ "#VALUE!" };
    }
    // Return as string to preserve precision for double
    return LiteralValue{ std::to_string(sum / count) };
}

LiteralValue CellEvaluator::evalMIN(const std::vector<FormulaParam>& args) {
    // Must accept exactly 1 argument, which must be a range
    if (args.size() != 1 || !std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = expandRange(std::get<AddressRange>(args[0]));
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }

    double minVal = std::numeric_limits<double>::max();
    bool foundNumeric = false;

    for (const auto& lv : values) {
        double num = getNumericValue(lv);
        if (!std::isnan(num)) {
            minVal = std::min(minVal, num);
            foundNumeric = true;
        }
    }

    if (!foundNumeric) { // No numeric values found
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ static_cast<int>(minVal) }; // Convert to int if whole number
}

LiteralValue CellEvaluator::evalMAX(const std::vector<FormulaParam>& args) {
    // Must accept exactly 1 argument, which must be a range
    if (args.size() != 1 || !std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = expandRange(std::get<AddressRange>(args[0]));
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }

    double maxVal = std::numeric_limits<double>::lowest();
    bool foundNumeric = false;

    for (const auto& lv : values) {
        double num = getNumericValue(lv);
        if (!std::isnan(num)) {
            maxVal = std::max(maxVal, num);
            foundNumeric = true;
        }
    }

    if (!foundNumeric) { // No numeric values found
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ static_cast<int>(maxVal) }; // Convert to int if whole number
}

LiteralValue CellEvaluator::evalCONCAT(const std::vector<FormulaParam>& args) {
    // Must have exactly 2 parameters: first a range, second a literal value (delimiter)
    if (args.size() != 2 || !std::holds_alternative<AddressRange>(args[0]) || !std::holds_alternative<LiteralValue>(args[1])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> rangeValues = expandRange(std::get<AddressRange>(args[0]));
    if (containsErrorLiteral(rangeValues)) {
        return LiteralValue{ "#VALUE!" };
    }

    std::string delimiter = getStringValue(std::get<LiteralValue>(args[1]));
    std::string result = "";
    bool firstValue = true;

    for (const auto& lv : rangeValues) {
        std::string currentString = getStringValue(lv);
        if (!currentString.empty()) { // Skip empty cells
            if (!firstValue) {
                result += delimiter;
            }
            result += currentString;
            firstValue = false;
        }
    }

    if (result.empty() && !rangeValues.empty()) { // If all cells in range were empty
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ result };
}

LiteralValue CellEvaluator::evalSUBSTR(const std::vector<FormulaParam>& args) {
    // Must accept exactly 3 arguments: text, start, length. Text cannot be a range.
    if (args.size() != 3 || std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = flattenArgs(args); // Flatten to resolve references
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }

    std::string text = getStringValue(values[0]);
    double startDouble = getNumericValue(values[1]);
    double lengthDouble = getNumericValue(values[2]);

    // Check if start/length are numeric and whole numbers
    if (std::isnan(startDouble) || startDouble != static_cast<int>(startDouble) ||
        std::isnan(lengthDouble) || lengthDouble != static_cast<int>(lengthDouble)) {
        return LiteralValue{ "#VALUE!" };
    }

    int start = static_cast<int>(startDouble);
    int length = static_cast<int>(lengthDouble);

    // Specific error conditions from requirements
    if (start < 1 || length <= 0) {
        return LiteralValue{ "#VALUE!" };
    }
    // Adjust for 0-based indexing for std::string::substr
    start--;

    if (start >= text.length()) { // Start index too large
        return LiteralValue{ "#VALUE!" };
    }
    if (start + length > text.length()) { // Length extends beyond string
        return LiteralValue{ "#VALUE!" };
    }

    return LiteralValue{ text.substr(start, length) };
}

LiteralValue CellEvaluator::evalLEN(const std::vector<FormulaParam>& args) {
    // Must accept exactly 1 argument, and it cannot be a range
    if (args.size() != 1 || std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = flattenArgs(args); // Flatten to resolve references
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }

    std::string text = getStringValue(values[0]);
    return LiteralValue{ static_cast<int>(text.length()) };
}

LiteralValue CellEvaluator::evalCOUNT(const std::vector<FormulaParam>& args) {
    // Must accept exactly 1 argument, which must be a range
    if (args.size() != 1 || !std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = expandRange(std::get<AddressRange>(args[0]));
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }

    int count = 0;
    for (const auto& lv : values) {
        // Count non-empty strings, non-NaN numbers, and booleans
        bool is_empty_string = std::holds_alternative<std::string>(lv.value) && std::get<std::string>(lv.value).empty();
        // getNumericValue returns NaN for non-numeric strings, int is never NaN
        bool is_numeric_and_nan = std::holds_alternative<std::string>(lv.value) && std::isnan(getNumericValue(lv));

        if (!is_empty_string && !is_numeric_and_nan) {
            count++;
        }
    }
    return LiteralValue{ count };
}
