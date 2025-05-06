#include "CellText.h"

std::string CellText::displayValue(const Table&) const {
	return _literalValue;
}

double CellText::numericalValue(const Table&) const {
	return 0.0;
}
