#include "CellText.h"

std::string CellText::displayValue(const Cells& cells) const {
	return _literalValue;
}

double CellText::numericalValue(const Cells& cells) const {
	return 0.0;
}
