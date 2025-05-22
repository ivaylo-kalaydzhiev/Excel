#include "CellReference.h"

std::string CellReference::displayValue(const Cells& cells) const {
	CellAddress address = CellAddress::fromString(_literalValue);
	sp<CellValue> value = cells.at(address);
	return value->displayValue(cells);
}

double CellReference::numericalValue(const Cells& cells) const {
	CellAddress address = CellAddress::fromString(_literalValue);
	sp<CellValue> value = cells.at(address);
	return value->numericalValue(cells);
}
