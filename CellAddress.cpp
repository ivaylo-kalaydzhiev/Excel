#include "CellAddress.h"
#include <stdexcept>
#include <cctype>

CellAddress CellAddress::fromString(const std::string& str) {
    if (str.length() < 2) throw std::invalid_argument("Invalid cell address");

    size_t col = 0;
    size_t i = 0;
    while (i < str.size() && std::isalpha(str[i])) {
        col = col * 26 + (std::toupper(str[i]) - 'A' + 1);
        ++i;
    }
    if (col == 0 || i >= str.size()) throw std::invalid_argument("Invalid cell address");

    int rowNumber = std::stoi(str.substr(i));
    if (rowNumber < 1) throw std::invalid_argument("Row must be >= 1");

    return CellAddress{ static_cast<size_t>(rowNumber - 1), col };
}

std::string CellAddress::toString() const {
    std::string colStr;
    size_t col = column;
    while (col > 0) {
        --col;
        colStr = static_cast<char>('A' + col % 26) + colStr;
        col /= 26;
    }
    return colStr + std::to_string(row + 1);
}

bool CellAddress::operator==(const CellAddress& other) const {
    return this->row == other.row && this->column == other.column;
}