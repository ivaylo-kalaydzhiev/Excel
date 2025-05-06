#include "CellAddress.h"
#include <cctype>
#include <algorithm>

CellAddress::CellAddress(int row, int col) : _column(col), _row(row) {
    if (col < 0 || row < 0) {
        throw std::invalid_argument("Column and row indices must be non-negative");
    }
}

CellAddress CellAddress::fromString(const std::string& str) {
    size_t i = 0;
    while (i < str.size() && std::isalpha(str[i])) ++i;

    if (i == 0 || i == str.size()) {
        throw std::invalid_argument("Invalid cell address: " + str);
    }

    std::string rowStr = str.substr(0, i);
    std::string colStr = str.substr(i);

    for (char c : colStr) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("Invalid column in cell address: " + str);
        }
    }

    int row = parseRow(rowStr);
    int col = std::stoi(colStr) - 1;

    return CellAddress(row, col);
}

std::string CellAddress::toString() const {
    return rowToString(_row) + std::to_string(_column + 1);
}

int CellAddress::getRow() const { 
    return _row; 
}
int CellAddress::getColumn() const { 
    return _column; 
}

int CellAddress::parseRow(const std::string& rowStr) {
    int result = 0;
    for (char c : rowStr) {
        if (!std::isalpha(c) || !std::isupper(c)) {
            throw std::invalid_argument("Invalid row letters: " + rowStr);
        }
        result *= 26;
        result += c - 'A' + 1;
    }
    return result - 1;
}

std::string CellAddress::rowToString(int row) {
    std::string result;
    ++row;
    while (row > 0) {
        row -= 1;
        result.insert(result.begin(), 'A' + (row % 26));
        row /= 26;
    }
    return result;
}