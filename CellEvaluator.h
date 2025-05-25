#pragma once

#include <vector>
#include <string>
#include "CellAddress.h"
#include "TableModel.h"
#include "CellValue.h"
#include "Event.h"

class CellEvaluator {
public:
    CellEvaluator(const TableModel& model);

    // Evaluate a single cell value in context
    std::string evaluate(const CellValue& cellValue);

private:
    const TableModel& model;
    
    LiteralValue resolve(const CellValue& value);
    LiteralValue evaluateFormula(const FormulaValue& formula);
    bool containsErrorLiteral(const std::vector<LiteralValue>& values);

    LiteralValue evalSUM(const std::vector<FormulaParam>& args);
    LiteralValue evalAVERAGE(const std::vector<FormulaParam>& args);
    LiteralValue evalMIN(const std::vector<FormulaParam>& args);
    LiteralValue evalMAX(const std::vector<FormulaParam>& args);
    LiteralValue evalCONCAT(const std::vector<FormulaParam>& args);
    LiteralValue evalSUBSTR(const std::vector<FormulaParam>& args);
    LiteralValue evalLEN(const std::vector<FormulaParam>& args);
    LiteralValue evalCOUNT(const std::vector<FormulaParam>& args);

    std::vector<LiteralValue> flattenArgs(const std::vector<FormulaParam>& args);
    std::vector<LiteralValue> expandRange(const AddressRange& range);
};