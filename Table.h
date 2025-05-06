#pragma once

#include <unordered_map>
#include "CellAddress.h"
#include "CellValue.h"

struct Table {
    int rows;
    int cols;

    std::unordered_map<CellAddress, CellValue> cells;
};