#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include "TableViewModel.h"

class TableView {
public:
    explicit TableView(const TableViewModel& viewModel);

    void redraw() const;

private:
    const TableViewModel& viewModel;

    // Helper methods for drawing
    void clearConsole() const;
    void drawTable() const;
    void drawHeader() const;
    void drawSeparatorLine(const std::vector<int>& columnWidths) const;
    void drawRow(int rowIndex, const std::vector<int>& columnWidths) const;

    // Utility methods
    std::vector<int> calculateColumnWidths() const;
    std::string formatCellContent(const std::string& content, int width, Alignment alignment) const;
    std::string getColumnLabel(int colIndex) const;
    std::string getCellDisplayValue(int row, int col) const;
    int getVisibleRows() const;
    int getVisibleCols() const;
};