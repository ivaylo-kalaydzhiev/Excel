#pragma once

#include "TableConfiguration.h"
#include "TableModel.h"
#include "DisplayableTableModel.h"
#include "EventParser.h"
#include "CellEvaluator.h"

class TableViewModel {
public:
    TableViewModel(TableConfiguration config, TableModel tableModel);

    void handle(const Event& event);

    const TableConfiguration& getConfiguration() const;
    const DisplayableTableModel& getDisplayableTableModel() const;

private:
    TableConfiguration configuration;
    TableModel tableModel;
    DisplayableTableModel displayableTableModel;

    void updateDisplayableCell(const CellAddress& address);
    void updateAllDisplayableCells();
};
