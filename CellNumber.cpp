#include "CellNumber.h"

std::string CellNumber::displayValue(const Cells& cells) const {
	return _literalValue;
}

double CellNumber::numericalValue(const Cells& cells) const {
	return std::stod(_literalValue);
}
