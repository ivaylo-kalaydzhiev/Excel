#pragma once

#include <string>
#include <variant>
#include <vector>
#include "CellAddress.h"
#include "Event.h"

struct CellValue {
    std::variant<std::string, double, bool, CellAddress, FormulaValue> value;

    bool isRaw() const;
    bool isReference() const;
    bool isFormula() const;
};
