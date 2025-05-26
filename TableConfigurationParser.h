#pragma once

#include <string>
#include "TableConfiguration.h"

class TableConfigurationParser {
public:
    // Constructor will load and validate the configuration
    TableConfigurationParser(const std::string& filePath);

    // Access the parsed configuration
    const TableConfiguration& getConfig() const;

private:
    TableConfiguration config;

    void parseFile(const std::string& filePath);
    void validateAndSet(const std::string& name, const std::string& value);
    Alignment parseAlignment(const std::string& value);

    // Utility validation functions
    bool isPositiveInteger(const std::string& value) const;
    bool isBoolean(const std::string& value) const;
};
