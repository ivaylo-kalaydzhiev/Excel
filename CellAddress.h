#pragma once

#include <string>
#include <cstddef>

struct CellAddress {
    size_t row;    // 0-based row index
    size_t column; // 0-based column index

    static CellAddress fromString(const std::string& str);
    std::string toString() const;

    bool operator==(const CellAddress& other) const;
};

namespace std {
    template<>
    struct hash<CellAddress> {
        size_t operator()(const CellAddress& addr) const {
            return hash<size_t>()(addr.row) ^ (hash<size_t>()(addr.column) << 1);
        }
    };
}
