#pragma once

#include "Formula.h"

// - Event Types

struct OpenTableEvent {
    std::string configFileName;
    std::string tableName;
};

struct NewTableEvent {
    std::string configFileName;
};

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
    OpenTableEvent,
    NewTableEvent,
    InsertEvent,
    DeleteEvent,
    ReferenceEvent,
    FormulaEvent
>;
