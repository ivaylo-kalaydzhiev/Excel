#pragma once
#include "CellValue.h"

class CellNumber : public CellValue {
public:

	std::string displayValue(const Cells& cells) const override;
	double numericalValue(const Cells& cells) const override;
};
