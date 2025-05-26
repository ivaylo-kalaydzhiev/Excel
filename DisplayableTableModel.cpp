#include "DisplayableTableModel.h"

void DisplayableTableModel::setDisplayValue(const CellAddress& address, const std::string& value) {
    displayValues[address] = value;
}

void DisplayableTableModel::removeDisplayValue(const CellAddress& address) {
    displayValues.erase(address);
}

const std::string* DisplayableTableModel::getDisplayValue(const CellAddress& address) const {
    auto it = displayValues.find(address);
    return (it != displayValues.end()) ? &it->second : nullptr;
}

const DisplayableTableModel::DisplayMap& DisplayableTableModel::getAllDisplayValues() const {
    return displayValues;
}

size_t DisplayableTableModel::getRowCount() const {
    size_t rowMax = 0;
    for (const auto& val : displayValues) {
        rowMax = std::max(rowMax, val.first.row);
    }
    return rowMax + 1;
}

size_t DisplayableTableModel::getColumnCount() const {
    size_t colMax = 0;
    for (const auto& val : displayValues) {
        colMax = std::max(colMax, val.first.column);
    }
    return colMax + 1;
}