#pragma once

#include <string>
#include "Table.h"

class CellValue {
public:
    CellValue(std::string literal);

    virtual ~CellValue() = default;

    virtual std::string displayValue(const Table& table) const = 0;
    virtual double numericalValue(const Table& table) const = 0;
    virtual void evaluateIn(const Table& table) = 0;

    const std::string& literalValue() const;

protected:
    std::string _literalValue;
};
