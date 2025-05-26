#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>

#include "TableConfigurationParser.h"
#include "TableConfiguration.h"
#include "TableParser.h"
#include "TableModel.h"
#include "TableViewModel.h"
#include "TableView.h"
#include "EventParser.h"
#include "Event.h"

// - HELPERS

void printWelcomeMessage() {
    std::cout << "===========================================\n";
    std::cout << "    Excel-like Console Spreadsheet\n";
    std::cout << "===========================================\n";
    std::cout << "Commands:\n";
    std::cout << "  {cell} insert {value}      - Insert value into cell\n";
    std::cout << "  {cell} delete              - Delete cell content\n";
    std::cout << "  {cell}={reference}         - Create cell reference\n";
    std::cout << "  {cell}=SUM(...)            - Create formula\n";
    std::cout << "  open {tableName} {config}  - Load existing table\n";
    std::cout << "  new {config}               - Create new table\n";
    std::cout << "  exit                       - Exit application\n";
    std::cout << "===========================================\n\n";
}

std::string promptForInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

TableConfiguration loadConfiguration(const std::string& configFileName) {
    try {
        TableConfigurationParser parser(configFileName);
        return parser.getConfig();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to load configuration from '" + configFileName + "': " + e.what());
    }
}

TableModel loadTable(const std::string& tableFileName) {
    try {
        TableParser parser;
        return parser.load(tableFileName);
    }
    catch (const std::exception& e) {
        std::cout << "Note: Could not load table from '" << tableFileName << "'. Starting with empty table. (" << e.what() << ")\n";
        return TableModel();
    }
}

void runEventLoop(TableViewModel& viewModel, TableView& view, EventParser& eventParser, std::string& tableFileName) {
    std::cout << "Starting interactive mode. Type 'exit' to quit.\n\n";

    view.redraw();

    while (true) {
        try {
            std::string input = promptForInput("> ");
            if (input.empty()) continue;

            if (input == "exit") {
                if (tableFileName.empty()) {
                    tableFileName = promptForInput("Enter a filename to save your table: ");
                }
                TableParser::save(viewModel.getTableModel(), tableFileName);
                std::cout << "Table saved to '" << tableFileName << "'.\n";
                std::cout << "Goodbye!\n";
                break;
            }

            Event event = eventParser.parse(input);
            viewModel.handle(event);
            view.redraw();

        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::cout << "Please try again or type 'exit' to quit.\n\n";
        }
    }
}

void handleStartupCommand(const std::string& input, TableConfiguration& config, TableModel& tableModel, std::string& tableFileName) {
    EventParser parser;
    try {
        Event event = parser.parse(input);

        if (std::holds_alternative<OpenTableEvent>(event)) {
            const auto& openEvent = std::get<OpenTableEvent>(event);

            config = loadConfiguration(openEvent.configFileName);
            tableModel = loadTable(openEvent.tableName);
            tableFileName = openEvent.tableName;
            std::cout << "Successfully loaded table '" << openEvent.tableName << "' with configuration '" << openEvent.configFileName << "'\n\n";
        }
        else if (std::holds_alternative<NewTableEvent>(event)) {
            const auto& newEvent = std::get<NewTableEvent>(event);

            config = loadConfiguration(newEvent.configFileName);
            tableModel = TableModel();
            tableFileName = "";
            std::cout << "Successfully created new table with configuration '" << newEvent.configFileName << "'\n\n";
        }
        else {
            throw std::runtime_error("Invalid startup command. Use 'open {tableName} {configFileName}' or 'new {configFileName}'");
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to process startup command: " + std::string(e.what()));
    }
}

// - MAIN

int main() {
    try {
        printWelcomeMessage(); 
        std::string startupInput = promptForInput("Enter startup command (open/new): ");

        std::string tableFileName;

        TableConfiguration config;
        TableModel tableModel;
        handleStartupCommand(startupInput, config, tableModel, tableFileName);

        TableViewModel viewModel(config, tableModel);
        TableView view(viewModel);
        EventParser eventParser;
        runEventLoop(viewModel, view, eventParser, tableFileName);

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        std::cerr << "Application will terminate." << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        std::cerr << "Application will terminate." << std::endl;
        return 2;
    }

    return 0;
}