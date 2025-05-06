#include "CellBool.h"

std::string CellBool::displayValue(const Table&) const {
	return _literalValue;
}

double CellBool::numericalValue(const Table&) const {
	return _literalValue == "TRUE" ? 1.0 : 0.0;
}
