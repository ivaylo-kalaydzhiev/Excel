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