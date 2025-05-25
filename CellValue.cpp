#include "CellValue.h"

bool CellValue::isRaw() const {
    return std::holds_alternative<std::string>(value) ||
        std::holds_alternative<double>(value) ||
        std::holds_alternative<bool>(value);
}

bool CellValue::isReference() const {
    return std::holds_alternative<CellAddress>(value);
}

bool CellValue::isFormula() const {
    return std::holds_alternative<FormulaValue>(value);
}
