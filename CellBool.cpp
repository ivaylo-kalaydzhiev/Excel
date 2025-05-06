#include "CellBool.h"

std::string CellBool::displayValue(const Cells& cells) const {
	return _literalValue;
}

double CellBool::numericalValue(const Cells& cells) const {
	return _literalValue == "TRUE" ? 1.0 : 0.0;
}
