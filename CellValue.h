#pragma once

#include <string>
#include <unordered_map>
#include "CellAddress.h"

/// Abstract base class representing a value stored in a spreadsheet cell.
/// 
/// All derived cell types (number, boolean, text, formula, reference, range) must
/// implement methods to evaluate themselves in the context of a table and provide
/// both a displayable and numeric form.
class CellValue {
public:
    /// Constructs a CellValue with the given literal string.
    /// @param literal The raw input string provided by the user.
    explicit CellValue(std::string literal);

    virtual ~CellValue() = default;

    /// Returns the string to display in the UI after evaluation.
    ///
    /// This is typically a user-friendly representation of the cell's value.
    /// For example:
    /// - A number cell shows the number as a string.
    /// - A formula cell may show the evaluated result.
    /// - A reference may show the referenced cell's value.
    ///
    /// @param table The context in which the cell is being evaluated.
    /// @return A string representing the display value of the cell.
    virtual std::string displayValue(const std::unordered_map<CellAddress, CellValue>& cells) const = 0;

    /// Returns the numerical value of the cell after evaluation.
    ///
    /// This is used for calculations, formulas, etc. The meaning varies by type:
    /// - Number cells return the parsed number.
    /// - Boolean cells return 1 (true) or 0 (false).
    /// - Text, range, or invalid types return 0.
    /// - Formula cells return the evaluated result.
    ///
    /// @param table The context in which the cell is being evaluated.
    /// @return A double representing the numeric value of the cell.
    virtual double numericalValue(const std::unordered_map<CellAddress, CellValue>& cells) const = 0;

protected:
    /// The raw user-provided string representing this cell's input.
    std::string _literalValue;
};
