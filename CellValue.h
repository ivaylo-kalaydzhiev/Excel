#pragma once

#include <string>
#include <variant>
#include <vector>
#include "CellAddress.h"
#include "Event.h"

struct CellValue {
    std::variant<LiteralValue, CellAddress, FormulaValue> value;

    bool isLiteral() const;
    bool isReference() const;
    bool isFormula() const;
};
