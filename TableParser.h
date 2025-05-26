#pragma once

#include "TableModel.h"
#include <string>
#include <optional>

class TableParser {
public:
    static bool save(const TableModel& table, const std::string& filename);
    static TableModel load(const std::string& filename);

private:
    static std::string serializeLiteralValue(const LiteralValue& lv);
    static std::optional<LiteralValue> deserializeLiteralValue(const std::string& s);
    
    static std::string serializeFormulaParam(const FormulaParam& fp);
    static std::optional<FormulaParam> deserializeFormulaParam(const std::string& s);
    
    static std::string serializeFormulaValue(const FormulaValue& fv);
    static std::optional<FormulaValue> deserializeFormulaValue(const std::string& s);
    
    static std::string serializeCellValue(const CellValue& cv);
    static std::optional<CellValue> deserializeCellValue(const std::string& s);
};