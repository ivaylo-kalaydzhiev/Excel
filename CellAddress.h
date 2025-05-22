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

    bool operator==(const CellAddress& other) const {
        return _row == other._row && _column == other._column;
    }

private:
    // Rows and columns are stored as zero-based indexing
    int _column;
    int _row;

    static int parseRow(const std::string& rowStr);
    static std::string rowToString(int row);
};

// Hash function for CellAddress
namespace std {
    template <>
    struct hash<CellAddress> {
        size_t operator()(const CellAddress& addr) const {
            return std::hash<int>()(addr.getRow()) ^ (std::hash<int>()(addr.getColumn()) << 1);
        }
    };
}