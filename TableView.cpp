#include "TableView.h"
#include "TableViewModel.h"
#include "TableConfiguration.h"
#include "DisplayableTableModel.h"
#include "CellAddress.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

TableView::TableView(const TableViewModel& viewModel)
    : viewModel(viewModel) {
}

void TableView::redraw() const {
    const auto& config = viewModel.getConfiguration();

    if (config.clearConsoleAfterCommand) {
        clearConsole();
    }

    drawTable();
}

void TableView::clearConsole() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void TableView::drawTable() const {
    std::vector<int> columnWidths = calculateColumnWidths();

    drawHeader();
    drawSeparatorLine(columnWidths);

    int visibleRows = getVisibleRows();
    for (int row = 0; row < visibleRows; ++row) {
        drawRow(row, columnWidths);
        if (row < visibleRows - 1) {
            drawSeparatorLine(columnWidths);
        }
    }

    drawSeparatorLine(columnWidths);
}

void TableView::drawHeader() const {
    std::vector<int> columnWidths = calculateColumnWidths();
    int visibleCols = getVisibleCols();

    // Draw top border
    std::cout << "|";
    for (int i = 0; i < 4; ++i) {
        std::cout << "-";
    }
    std::cout << "|";

    for (int col = 0; col < visibleCols; ++col) {
        for (int i = 0; i < columnWidths[col]; ++i) {
            std::cout << "-";
        }
        std::cout << "|";
    }
    std::cout << std::endl;

    // Draw header row with column numbers
    std::cout << "|    |";
    for (int col = 0; col < visibleCols; ++col) {
        std::string colLabel = std::to_string(col + 1);
        std::string formatted = formatCellContent(colLabel, columnWidths[col], Alignment::Center);
        std::cout << formatted << "|";
    }
    std::cout << std::endl;
}

void TableView::drawSeparatorLine(const std::vector<int>& columnWidths) const {
    int visibleCols = getVisibleCols();

    std::cout << "|";
    for (int i = 0; i < 4; ++i) {
        std::cout << "-";
    }
    std::cout << "|";

    for (int col = 0; col < visibleCols; ++col) {
        for (int i = 0; i < columnWidths[col]; ++i) {
            std::cout << "-";
        }
        std::cout << "|";
    }
    std::cout << std::endl;
}

void TableView::drawRow(int rowIndex, const std::vector<int>& columnWidths) const {
    int visibleCols = getVisibleCols();
    const auto& config = viewModel.getConfiguration();

    // Draw row label (A, B, C, etc.)
    std::string rowLabel = getRowLabel(rowIndex);
    std::cout << "| " << std::setw(2) << std::left << rowLabel << " |";

    // Draw cells in the row
    for (int col = 0; col < visibleCols; ++col) {
        std::string cellContent = getCellDisplayValue(rowIndex, col);
        std::string formatted = formatCellContent(cellContent, columnWidths[col], config.initialAlignment);
        std::cout << formatted << "|";
    }
    std::cout << std::endl;
}

std::vector<int> TableView::calculateColumnWidths() const {
    const auto& config = viewModel.getConfiguration();
    int visibleCols = getVisibleCols();
    std::vector<int> widths(visibleCols);

    if (config.autoFit) {
        // Calculate width based on content
        for (int col = 0; col < visibleCols; ++col) {
            // Minimum width for column number
            int maxWidth = 3;

            int visibleRows = getVisibleRows();
            for (int row = 0; row < visibleRows; ++row) {
                std::string content = getCellDisplayValue(row, col);
                maxWidth = max(maxWidth, content.length());
            }

            // Add padding
            widths[col] = maxWidth + 2;
        }
    }
    else {
        // Use fixed width based on visibleCellSymbols + padding
        int fixedWidth = config.visibleCellSymbols + 2;
        for (int col = 0; col < visibleCols; ++col) {
            widths[col] = fixedWidth;
        }
    }

    return widths;
}

std::string TableView::formatCellContent(const std::string& content, int width, Alignment alignment) const {
    std::string truncated = content;
    // Account for padding
    int availableWidth = width - 2;

    if (static_cast<int>(truncated.length()) > availableWidth) {
        truncated = truncated.substr(0, availableWidth);
    }

    std::ostringstream oss;
    // Left padding
    oss << " ";

    switch (alignment) {
    case Alignment::Left:
        oss << std::left << std::setw(availableWidth) << truncated;
        break;
    case Alignment::Center:
    {
        int totalPadding = availableWidth - static_cast<int>(truncated.length());
        int leftPadding = totalPadding / 2;
        int rightPadding = totalPadding - leftPadding;

        for (int i = 0; i < leftPadding; ++i) oss << " ";
        oss << truncated;
        for (int i = 0; i < rightPadding; ++i) oss << " ";
    }
    break;
    case Alignment::Right:
        oss << std::right << std::setw(availableWidth) << truncated;
        break;
    }

    // Right padding
    oss << " ";
    return oss.str();
}

std::string TableView::getRowLabel(int rowIndex) const {
    // Convert 0-based index to Excel-style letters (A, B, C, ..., Z, AA, AB, ...)
    std::string label;
    int index = rowIndex;

    do {
        label = char('A' + (index % 26)) + label;
        index = index / 26 - 1;
    } while (index >= 0);

    return label;
}

std::string TableView::getCellDisplayValue(int row, int col) const {
    const auto& displayModel = viewModel.getDisplayableTableModel();
    CellAddress address{ row, col };

    const std::string* value = displayModel.getDisplayValue(address);
    return value ? *value : "";
}

int TableView::getVisibleRows() const {
    const auto& config = viewModel.getConfiguration();
    int initialRows = config.initialTableRows;
    int maxRows = config.maxTableRows;
    int actualRows = viewModel.getDisplayableTableModel().getRowCount();
    if (actualRows < initialRows) {
        return initialRows;
    }
    else if (actualRows > maxRows) {
        return maxRows;
    }
    else {
        return actualRows;
    }
}

int TableView::getVisibleCols() const {
    const auto& config = viewModel.getConfiguration();
    int initialCols = config.initialTableRows;
    int maxCols = config.maxTableRows;
    int actualCols = viewModel.getDisplayableTableModel().getColumnCount();
    if (actualCols < initialCols) {
        return initialCols;
    }
    else if (actualCols > maxCols) {
        return maxCols;
    }
    else {
        return actualCols;
    }
}