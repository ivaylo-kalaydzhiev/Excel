#include "TableViewModel.h"

TableViewModel::TableViewModel(TableConfiguration config, TableModel tableModel)
    : configuration(std::move(config)), tableModel(std::move(tableModel)) {
    updateAllDisplayableCells();
}

void TableViewModel::handle(const Event& event) {
    std::visit([this](const auto& e) {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, InsertEvent>) {
            //tableModel.setCellValue(e.target, CellValue{ e.value });
            updateDisplayableCell(e.target);
        }
        else if constexpr (std::is_same_v<T, DeleteEvent>) {
            tableModel.removeCellValue(e.target);
            displayableTableModel.removeDisplayValue(e.target);
        }
        else if constexpr (std::is_same_v<T, ReferenceEvent>) {
            tableModel.setCellValue(e.target, CellValue{ e.source });
            updateDisplayableCell(e.target);
        }
        else if constexpr (std::is_same_v<T, FormulaEvent>) {
            tableModel.setCellValue(e.target, CellValue{ FormulaValue{e.formula, e.params} });
            updateDisplayableCell(e.target);
        }
        }, event);
}

const TableConfiguration& TableViewModel::getConfiguration() const {
    return configuration;
}

const DisplayableTableModel& TableViewModel::getDisplayableTableModel() const {
    return displayableTableModel;
}

void TableViewModel::updateDisplayableCell(const CellAddress& address) {
    const CellValue* cellValue = tableModel.getCellValue(address);
    if (cellValue) {
        CellEvaluator evaluator(tableModel);
        displayableTableModel.setDisplayValue(address, evaluator.evaluate(*cellValue));
    }
}

void TableViewModel::updateAllDisplayableCells() {
    const auto& allCells = tableModel.getAllCells();
    CellEvaluator evaluator(tableModel);

    for (const auto& [address, value] : allCells) {
        displayableTableModel.setDisplayValue(address, evaluator.evaluate(value));
    }
}
