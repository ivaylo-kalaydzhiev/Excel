#include "CellValue.h"

bool CellValue::isLiteral() const {
    return std::holds_alternative<LiteralValue>(value);
}

bool CellValue::isReference() const {
    return std::holds_alternative<CellAddress>(value);
}

bool CellValue::isFormula() const {
    return std::holds_alternative<FormulaValue>(value);
}
