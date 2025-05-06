#include "CellBool.h"

std::string CellBool::displayValue(const std::unordered_map<CellAddress, CellValue>& cells) const {
	return _literalValue;
}

double CellBool::numericalValue(const std::unordered_map<CellAddress, CellValue>& cells) const {
	return _literalValue == "TRUE" ? 1.0 : 0.0;
}
