#include "CellAddress.h"
#include <stdexcept>
#include <cctype>

CellAddress CellAddress::fromString(const std::string& str) {
    size_t i = 0;

    // Extract row (letters)
    while (i < str.size() && std::isalpha(str[i])) {
        ++i;
    }

    if (i == 0 || i == str.size()) {
        throw std::invalid_argument("Invalid cell address format: " + str);
    }

    std::string rowStr = str.substr(0, i);
    std::string colStr = str.substr(i);

    // Convert row (letters) to 0-based index
    size_t row = 0;
    for (char ch : rowStr) {
        if (!std::isalpha(ch)) {
            throw std::invalid_argument("Invalid row characters in: " + str);
        }
        row = row * 26 + (std::toupper(ch) - 'A' + 1);
    }
    row -= 1; // Convert to 0-based

    // Convert column (number) to 0-based index
    size_t column = std::stoul(colStr) - 1;

    return CellAddress{ row, column };
}

std::string CellAddress::toString() const {
    size_t r = row + 1; // Convert from 0-based to 1-based
    std::string rowStr;

    while (r > 0) {
        r -= 1;
        rowStr.insert(rowStr.begin(), 'A' + (r % 26));
        r /= 26;
    }

    return rowStr + std::to_string(column + 1); // Convert back to 1-based
}

bool CellAddress::operator==(const CellAddress& other) const {
    return this->row == other.row && this->column == other.column;
}