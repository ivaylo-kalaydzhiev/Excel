#pragma once

#include <unordered_map>
#include "CellAddress.h"
#include "CellValue.h"

class TableModel {
public:
    using CellMap = std::unordered_map<CellAddress, CellValue>;

    void setCellValue(const CellAddress& address, const CellValue& value);
    void removeCellValue(const CellAddress& address);
    const CellValue* getCellValue(const CellAddress& address) const;
    const CellMap& getAllCells() const;

private:
    CellMap cells;
};