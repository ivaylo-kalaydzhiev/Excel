#pragma once
#include "CellValue.h"

class CellBool : public CellValue {
public:

	std::string displayValue(const std::unordered_map<CellAddress, CellValue>& cells) const override;
	double numericalValue(const std::unordered_map<CellAddress, CellValue>& cells) const override;
};
