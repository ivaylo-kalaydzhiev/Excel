#include "TableViewModel.h"

TableViewModel::TableViewModel(TableConfiguration config, TableModel tableModel)
    : configuration(std::move(config)), tableModel(std::move(tableModel)) {
    updateAllDisplayableCells();
}

// We need to make sure we update both the TableModel and all of the DisplayableTableModel on each event handled
void TableViewModel::handle(const Event& event) {
    if (auto e = std::get_if<InsertEvent>(&event)) {
        tableModel.setCellValue(e->target, CellValue{ e->value } );
        updateAllDisplayableCells();
    }
    else if (auto e = std::get_if<DeleteEvent>(&event)) {
        tableModel.removeCellValue(e->target);
        displayableTableModel.removeDisplayValue(e->target);
        updateAllDisplayableCells();
    }
    else if (auto e = std::get_if<ReferenceEvent>(&event)) {
        tableModel.setCellValue(e->target, CellValue{ e->source });
        updateAllDisplayableCells();
    }
    else if (auto e = std::get_if<FormulaEvent>(&event)) {
        tableModel.setCellValue(e->target, CellValue{ FormulaValue{e->formula, e->params} });
        updateAllDisplayableCells();
    }
}

const TableConfiguration& TableViewModel::getConfiguration() const {
    return configuration;
}

const TableModel& TableViewModel::getTableModel() const {
    return tableModel;
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
