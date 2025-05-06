#include "CellNumber.h"

std::string CellNumber::displayValue(const std::unordered_map<CellAddress, CellValue>& cells) const {
	return _literalValue;
}

double CellNumber::numericalValue(const std::unordered_map<CellAddress, CellValue>& cells) const {
	return std::stod(_literalValue);
}
