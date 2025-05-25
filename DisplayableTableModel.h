#pragma once

#include <unordered_map>
#include <string>
#include "CellAddress.h"

class DisplayableTableModel {
public:
    using DisplayMap = std::unordered_map<CellAddress, std::string>;

    void setDisplayValue(const CellAddress& address, const std::string& value);
    void removeDisplayValue(const CellAddress& address);
    const std::string* getDisplayValue(const CellAddress& address) const;
    const DisplayMap& getAllDisplayValues() const;

private:
    DisplayMap displayValues;
};