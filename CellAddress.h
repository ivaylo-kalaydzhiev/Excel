#pragma once

#include <string>
#include <stdexcept>

class CellAddress {
public:
    CellAddress(int row, int col);
    static CellAddress fromString(const std::string& str);
    std::string toString() const;

    int getRow() const;
    int getColumn() const;

private:
    // Rows and columns are stored as zero-based indexing
    int _column;
    int _row;

    static int parseRow(const std::string& rowStr);
    static std::string rowToString(int row);
};