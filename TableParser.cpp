#include "TableParser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <variant>

// - Inerface

bool TableParser::save(const TableModel& table, const std::string& filename) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file for saving: " << filename << std::endl;
        return false;
    }

    for (const auto& pair : table.getAllCells()) {
        outputFile << pair.first.toString() << "=" << serializeCellValue(pair.second) << std::endl;
    }

    outputFile.close();
    return true;
}

TableModel TableParser::load(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening file for loading: " + filename);
    }

    TableModel table;
    std::string line;
    while (getline(inputFile, line)) {
        size_t equalsPos = line.find('=');
        if (equalsPos != std::string::npos) {
            std::string addressStr = line.substr(0, equalsPos);
            std::string valueStr = line.substr(equalsPos + 1);
            try {
                CellAddress address = CellAddress::fromString(addressStr);
                if (auto cell_value = deserializeCellValue(valueStr)) {
                    table.setCellValue(address, *cell_value);
                }
                else {
                    std::cerr << "Warning: Could not deserialize value: " << valueStr << " for cell " << addressStr << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Warning: Could not parse line: " << line << " (" << e.what() << ")" << std::endl;
            }
        }
        else {
            std::cerr << "Warning: Invalid line format: " << line << std::endl;
        }
    }

    inputFile.close();
    return table;
}

// - Literal Values

std::string TableParser::serializeLiteralValue(const LiteralValue& lv) {
    if (auto val = std::get_if<double>(&lv.value)) {
        return "number:" + std::to_string(*val);
    }
    else if (auto val = std::get_if<bool>(&lv.value)) {
        return "bool:" + std::string(*val ? "true" : "false");
    }
    else if (auto val = std::get_if<std::string>(&lv.value)) {
        return "string:" + *val;
    }
    return "";
}

std::optional<LiteralValue>TableParser::deserializeLiteralValue(const std::string& s) {
    if (s.rfind("number:", 0) == 0) {
        try {
            return LiteralValue{ stod(s.substr(7)) };
        }
        catch (...) {
            return std::nullopt;
        }
    }
    else if (s.rfind("bool:", 0) == 0) {
        std::string valStr = s.substr(5);
        if (valStr == "true") return LiteralValue{ true };
        if (valStr == "false") return LiteralValue{ false };
        return std::nullopt;
    }
    else if (s.rfind("string:", 0) == 0) {
        return LiteralValue{ s.substr(7) };
    }
    return std::nullopt;
}

// - FormulaParam

std::string TableParser::serializeFormulaParam(const FormulaParam& fp) {
    if (auto val = std::get_if<LiteralValue>(&fp)) {
        return "literal:" + serializeLiteralValue(*val);
    }
    else if (auto val = std::get_if<CellAddress>(&fp)) {
        return "cell:" + val->toString();
    }
    else if (auto val = std::get_if<AddressRange>(&fp)) {
        return "range:" + val->start.toString() + "-" + val->end.toString();
    }
    return "";
}

std::optional<FormulaParam> TableParser::deserializeFormulaParam(const std::string& s) {
    if (s.rfind("literal:", 0) == 0) {
        if (auto lv = deserializeLiteralValue(s.substr(8))) {
            return *lv;
        }
    }
    else if (s.rfind("cell:", 0) == 0) {
        try {
            return CellAddress::fromString(s.substr(5));
        }
        catch (...) {
            return std::nullopt;
        }
    }
    else if (s.rfind("range:", 0) == 0) {
        size_t dashPos = s.find('-');
        if (dashPos != std::string::npos) {
            try {
                CellAddress start = CellAddress::fromString(s.substr(6, dashPos - 6));
                CellAddress end = CellAddress::fromString(s.substr(dashPos + 1));
                return AddressRange{ start, end };
            }
            catch (...) {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

// - FormulaValue

std::string TableParser::serializeFormulaValue(const FormulaValue& fv) {
    std::string paramsStr;
    for (size_t i = 0; i < fv.parameters.size(); ++i) {
        paramsStr += serializeFormulaParam(fv.parameters[i]);
        if (i < fv.parameters.size() - 1) {
            paramsStr += ",";
        }
    }
    std::string formulaTypeStr;
    switch (fv.type) {
    case FormulaType::SUM: formulaTypeStr = "SUM"; break;
    case FormulaType::AVERAGE: formulaTypeStr = "AVERAGE"; break;
    case FormulaType::MIN: formulaTypeStr = "MIN"; break;
    case FormulaType::MAX: formulaTypeStr = "MAX"; break;
    case FormulaType::CONCAT: formulaTypeStr = "CONCAT"; break;
    case FormulaType::SUBSTR: formulaTypeStr = "SUBSTR"; break;
    case FormulaType::LEN: formulaTypeStr = "LEN"; break;
    case FormulaType::COUNT: formulaTypeStr = "COUNT"; break;
    }
    return "formula:" + formulaTypeStr + "(" + paramsStr + ")";
}

std::optional<FormulaValue> TableParser::deserializeFormulaValue(const std::string& s) {
    if (s.rfind("formula:", 0) == 0) {
        std::string content = s.substr(8);
        size_t openParen = content.find('(');
        size_t closeParen = content.find(')');
        if (openParen != std::string::npos && closeParen != std::string::npos && openParen < closeParen) {
            std::string typeStr = content.substr(0, openParen);
            std::string paramsStr = content.substr(openParen + 1, closeParen - openParen - 1);

            FormulaType type;
            if (typeStr == "SUM") type = FormulaType::SUM;
            else if (typeStr == "AVERAGE") type = FormulaType::AVERAGE;
            else if (typeStr == "MIN") type = FormulaType::MIN;
            else if (typeStr == "MAX") type = FormulaType::MAX;
            else if (typeStr == "CONCAT") type = FormulaType::CONCAT;
            else if (typeStr == "SUBSTR") type = FormulaType::SUBSTR;
            else if (typeStr == "LEN") type = FormulaType::LEN;
            else if (typeStr == "COUNT") type = FormulaType::COUNT;
            else return std::nullopt;

            std::vector<FormulaParam> params;
            std::stringstream ss(paramsStr);
            std::string paramToken;
            while (getline(ss, paramToken, ',')) {
                if (auto param = deserializeFormulaParam(paramToken)) {
                    params.push_back(*param);
                }
                else {
                    return std::nullopt;
                }
            }
            return FormulaValue{ type, params };
        }
    }
    return std::nullopt;
}

// - CellValue

std::string TableParser::serializeCellValue(const CellValue& cv) {
    if (auto val = std::get_if<LiteralValue>(&cv.value)) {
        return serializeLiteralValue(*val);
    }
    else if (auto val = std::get_if<CellAddress>(&cv.value)) {
        return "reference:" + val->toString();
    }
    else if (auto val = std::get_if<FormulaValue>(&cv.value)) {
        return serializeFormulaValue(*val);
    }
    return "";
}

std::optional<CellValue> TableParser::deserializeCellValue(const std::string& s) {
    if (s.rfind("number:", 0) == 0 || s.rfind("bool:", 0) == 0 || s.rfind("string:", 0) == 0) {
        if (auto lv = deserializeLiteralValue(s)) {
            return CellValue{ *lv };
        }
    }
    else if (s.rfind("reference:", 0) == 0) {
        try {
            return CellValue{ CellAddress::fromString(s.substr(10)) };
        }
        catch (...) {
            return std::nullopt;
        }
    }
    else if (s.rfind("formula:", 0) == 0) {
        if (auto fv = deserializeFormulaValue(s)) {
            return CellValue{ *fv };
        }
    }
    return std::nullopt;
}