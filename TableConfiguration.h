#pragma once
#include "Alignment.h"

struct TableConfiguration {
    int initialTableRows;
    int initialTableCols;
    int maxTableRows;
    int maxTableCols;
    bool autoFit;
    int visibleCellSymbols;
    Alignment initialAlignment;
    bool clearConsoleAfterCommand;
};