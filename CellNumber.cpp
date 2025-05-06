#include "CellNumber.h"

std::string CellNumber::displayValue(const Table&) const {
	return _literalValue;
}

double CellNumber::numericalValue(const Table&) const {
	return std::stod(_literalValue);
}
