#include "CellEvaluator.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <variant>

CellEvaluator::CellEvaluator(const TableModel& model) : model(model) {}

// - HELPERS

std::string getStringValue(const LiteralValue& lv) {
    if (auto val = std::get_if<double>(&lv.value)) {
        return std::to_string(*val);
    }
    else if (auto val = std::get_if<bool>(&lv.value)) {
        return (*val) ? "TRUE" : "FALSE";
    }
    else if (auto val = std::get_if<std::string>(&lv.value)) {
        return *val;
    }
    else {
        // Should not happen with current LiteralValue types
        return "#VALUE!";
    }
}

double getNumericValue(const LiteralValue& lv) {
    if (auto val = std::get_if<bool>(&lv.value)) {
        return (*val) ? 1.0 : 0.0;
    }
    else if (auto val = std::get_if<double>(&lv.value)) {
        return *val;
    }
    else if (auto val = std::get_if<std::string>(&lv.value)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    // Should not reach here
    return std::numeric_limits<double>::quiet_NaN();
}

// - Public evaluation interface

std::string CellEvaluator::evaluate(const CellValue& cellValue) {
    try {
        LiteralValue result = resolve(cellValue);
        return getStringValue(result);
    }
    catch (const std::runtime_error& e) {
        return e.what();
    }
    catch (const std::invalid_argument& e) {
        return e.what();
    }
    catch (...) {
        return "#ERROR!";
    }
}

// - Private evaluation

LiteralValue CellEvaluator::resolve(const CellValue& value) {
    if (value.isLiteral()) {
        const LiteralValue& lv = std::get<LiteralValue>(value.value);
        return lv;
    }
    else if (value.isReference()) {
        const CellAddress& targetAddress = std::get<CellAddress>(value.value);
        const CellValue* targetCellValue = model.getCellValue(targetAddress);
        if (!targetCellValue) {
            // Reference to an empty or non-existent cell
            return LiteralValue{ "#REF!" };
        }
        // Recursive call to resolve the referenced cell's value
        return resolve(*targetCellValue);
    }
    else if (value.isFormula()) {
        const FormulaValue& formula = std::get<FormulaValue>(value.value);
        return evaluateFormula(formula);
    }

    // Should never come here
    return LiteralValue{ "#VALUE!" };
}

// - Formula Evaluation Helpers

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
            return LiteralValue{ "#NAME?" };
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
        if (auto val = std::get_if<LiteralValue>(&param)) {
            flattened.push_back(*val);
        }
        else if (auto val = std::get_if<CellAddress>(&param)) {
            const CellValue* cellValue = model.getCellValue(*val);
            if (cellValue) {
                flattened.push_back(resolve(*cellValue));
            }
            else {
                // Per requirements, empty cell evaluates to 0 for numeric context, or empty string for others
                // For now, push empty string, numeric functions will convert to NaN or 0 as needed
                flattened.push_back(LiteralValue{ "" });
            }
        }
        else if (auto val = std::get_if<AddressRange>(&param)) {
            std::vector<LiteralValue> rangeValues = expandRange(*val);
            flattened.insert(flattened.end(), rangeValues.begin(), rangeValues.end());
        }
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
    if (values.empty()) {
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

    if (!hasNumeric) {
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ sum };
}

LiteralValue CellEvaluator::evalAVERAGE(const std::vector<FormulaParam>& args) {
    std::vector<LiteralValue> values = flattenArgs(args);
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }
    if (values.empty()) {
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

    // So there were no numerical values
    if (count == 0) {
        return LiteralValue{ "#VALUE!" };
    }

    return LiteralValue{ sum / count };
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

    if (!foundNumeric) {
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ minVal };
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

    if (!foundNumeric) {
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ maxVal };
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
        if (!currentString.empty()) {
            if (!firstValue) {
                result += delimiter;
            }
            result += currentString;
            firstValue = false;
        }
    }

    if (result.empty() && !rangeValues.empty()) {
        return LiteralValue{ "#VALUE!" };
    }
    return LiteralValue{ result };
}

LiteralValue CellEvaluator::evalSUBSTR(const std::vector<FormulaParam>& args) {
    // Must accept exactly 3 arguments: text, start, length. Text cannot be a range.
    if (args.size() != 3 || std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = flattenArgs(args);
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

    // Start index too large
    if (start >= text.length()) {
        return LiteralValue{ "#VALUE!" };
    }

    // Length extends beyond string
    if (start + length > text.length()) {
        return LiteralValue{ "#VALUE!" };
    }

    return LiteralValue{ text.substr(start, length) };
}

LiteralValue CellEvaluator::evalLEN(const std::vector<FormulaParam>& args) {
    // Must accept exactly 1 argument, and it cannot be a range
    if (args.size() != 1 || std::holds_alternative<AddressRange>(args[0])) {
        return LiteralValue{ "#VALUE!" };
    }

    std::vector<LiteralValue> values = flattenArgs(args);
    if (containsErrorLiteral(values)) {
        return LiteralValue{ "#VALUE!" };
    }

    std::string text = getStringValue(values[0]);
    return LiteralValue{ static_cast<double>(text.length()) };
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
        bool isEmptyString = std::holds_alternative<std::string>(lv.value) && std::get<std::string>(lv.value).empty();
        // getNumericValue returns NaN for non-numeric strings, int is never NaN
        bool isNumericAndNan = std::holds_alternative<std::string>(lv.value) && std::isnan(getNumericValue(lv));

        if (!isEmptyString && !isNumericAndNan) {
            count++;
        }
    }
    return LiteralValue{ static_cast<double>(count) };
}
