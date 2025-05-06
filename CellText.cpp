#include "CellText.h"

std::string CellText::displayValue(const std::unordered_map<CellAddress, CellValue>& cells) const {
	return _literalValue;
}

double CellText::numericalValue(const std::unordered_map<CellAddress, CellValue>& cells) const {
	return 0.0;
}
