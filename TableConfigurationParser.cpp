#include "TableConfigurationParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <map>
#include <iostream>

TableConfigurationParser::TableConfigurationParser(const std::string& filePath) {
    parseFile(filePath);
}

const TableConfiguration& TableConfigurationParser::getConfig() const {
    return config;
}

void TableConfigurationParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::string line;
    std::map<std::string, bool> requiredProperties = {
        {"initialTableRows", false},
        {"initialTableCols", false},
        {"maxTableRows", false},
        {"maxTableCols", false},
        {"autoFit", false},
        {"visibleCellSymbols", false},
        {"initialAlignment", false},
        {"clearConsoleAfterCommand", false}
    };

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error("ABORTING! " + line + ": - Invalid format!");
        }

        std::string name = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);

        // Track required properties
        if (requiredProperties.find(name) != requiredProperties.end()) {
            requiredProperties[name] = true;
        }

        validateAndSet(name, value);
    }

    // Check for missing required properties
    for (const auto& prop : requiredProperties) {
        if (!prop.second) {
            throw std::runtime_error("ABORTING! " + prop.first + ": - Missing property!");
        }
    }
}

void TableConfigurationParser::validateAndSet(const std::string& name, const std::string& value) {
    if (name == "initialTableRows") {
        if (!isPositiveInteger(value)) {
            throw std::runtime_error("ABORTING! initialTableRows:" + value + " - Invalid value!");
        }
        config.initialTableRows = std::stoi(value);
    }
    else if (name == "initialTableCols") {
        if (!isPositiveInteger(value)) {
            throw std::runtime_error("ABORTING! initialTableCols:" + value + " - Invalid value!");
        }
        config.initialTableCols = std::stoi(value);
    }
    else if (name == "maxTableRows") {
        if (!isPositiveInteger(value)) {
            throw std::runtime_error("ABORTING! maxTableRows:" + value + " - Invalid value!");
        }
        config.maxTableRows = std::stoi(value);
    }
    else if (name == "maxTableCols") {
        if (!isPositiveInteger(value)) {
            throw std::runtime_error("ABORTING! maxTableCols:" + value + " - Invalid value!");
        }
        config.maxTableCols = std::stoi(value);
    }
    else if (name == "autoFit") {
        if (!isBoolean(value)) {
            throw std::runtime_error("ABORTING! autoFit:" + value + " - Invalid value!");
        }
        config.autoFit = (value == "true");
    }
    else if (name == "visibleCellSymbols") {
        if (!isPositiveInteger(value)) {
            throw std::runtime_error("ABORTING! visibleCellSymbols:" + value + " - Invalid value!");
        }
        config.visibleCellSymbols = std::stoi(value);
    }
    else if (name == "initialAlignment") {
        config.initialAlignment = parseAlignment(value);
    }
    else if (name == "clearConsoleAfterCommand") {
        if (!isBoolean(value)) {
            throw std::runtime_error("ABORTING! clearConsoleAfterCommand:" + value + " - Invalid value!");
        }
        config.clearConsoleAfterCommand = (value == "true");
    }
    else {
        // Allow unknown properties as mentioned in the config spec
        // Do nothing
    }
}

Alignment TableConfigurationParser::parseAlignment(const std::string& value) {
    if (value == "left") return Alignment::Left;
    else if (value == "center") return Alignment::Center;
    else if (value == "right")  return Alignment::Right;

    throw std::runtime_error("ABORTING! initialAlignment:" + value + " - Invalid value!");
}

bool TableConfigurationParser::isPositiveInteger(const std::string& value) const {
    if (value.empty()) return false;
    for (char c : value) {
        if (!std::isdigit(c)) return false;
    }
    return std::stoi(value) > 0;
}

bool TableConfigurationParser::isBoolean(const std::string& value) const {
    return value == "true" || value == "false";
}
