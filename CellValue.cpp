#include "CellValue.h"

CellValue::CellValue(std::string literal) : _literalValue(literal) {}

const std::string& CellValue::literalValue() const { return _literalValue; }
