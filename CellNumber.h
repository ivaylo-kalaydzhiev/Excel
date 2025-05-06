#pragma once
#include "CellValue.h"

class CellNumber : public CellValue {
public:

	std::string displayValue(const Table&) const override;
	double numericalValue(const Table&) const override;
};
