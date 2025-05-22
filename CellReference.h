#pragma once
#include "CellValue.h"

class CellReference : public CellValue {
public:

	std::string displayValue(const Cells& cells) const override;
	double numericalValue(const Cells& cells) const override;
};

