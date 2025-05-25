#pragma once

#include <string>
#include <variant>
#include <vector>
#include "CellAddress.h"

// - Formula Parameter

struct LiteralValue {
    std::variant<int, bool, std::string> value;
};

struct ReferenceValue {
    CellAddress target;
};

struct AddressRange {
    CellAddress start;
    CellAddress end;
};

using FormulaParam = std::variant<LiteralValue, CellAddress, AddressRange>;

// - Formula Type

enum class FormulaType {
    SUM, AVERAGE, MIN, MAX, CONCAT, SUBSTR, LEN, COUNT
};

// - Formula

struct FormulaValue {
    FormulaType type;
    std::vector<FormulaParam> parameters;
};

// - Event Types

struct InsertEvent {
    CellAddress target;
    LiteralValue value;
};

struct DeleteEvent {
    CellAddress target;
};

struct ReferenceEvent {
    CellAddress target;
    CellAddress source;
};

struct FormulaEvent {
    CellAddress target;
    FormulaType   formula;
    std::vector<FormulaParam> params;
};

// - Event

using Event = std::variant<
    InsertEvent,
    DeleteEvent,
    ReferenceEvent,
    FormulaEvent
>;
