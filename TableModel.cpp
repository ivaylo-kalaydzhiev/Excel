#include "TableModel.h"

void TableModel::setCellValue(const CellAddress& address, const CellValue& value) {
    cells[address] = value;
}

void TableModel::removeCellValue(const CellAddress& address) {
    cells.erase(address);
}

const CellValue* TableModel::getCellValue(const CellAddress& address) const {
    auto it = cells.find(address);
    return (it != cells.end()) ? &it->second : nullptr;
}

const TableModel::CellMap& TableModel::getAllCells() const {
    return cells;
}