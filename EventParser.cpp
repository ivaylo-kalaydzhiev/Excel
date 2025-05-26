#include "EventParser.h"
#include "CellAddress.h"
#include <sstream>
#include <regex>
#include <stdexcept>

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t");
    auto end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

static const std::regex openTableRegex(R"(open (\w+) (.+))");
static const std::regex newTableRegex(R"(new (.+))");
static const std::regex deleteRegex(R"(([A-Z]+[0-9]+) delete)");
static const std::regex referenceRegex(R"(([A-Z]+[0-9]+)=([A-Z]+[0-9]+))");
static const std::regex insertRegex(R"(([A-Z]+[0-9]+) insert (.+))");
static const std::regex formulaRegex(R"(([A-Z]+[0-9]+)=(\w+)\((.*)\))");
static const std::regex numberRegex(R"(^-?\d+(\.\d+)?$)");
static const std::regex addressRegex(R"(^[A-Z]+[0-9]+$)");
static const std::regex rangeRegex(R"(^[A-Z]+[0-9]+:[A-Z]+[0-9]+$)");


Event EventParser::parse(const std::string& input) {
    std::smatch match;

    // open table
    if (std::regex_match(input, match, openTableRegex)) {
        return OpenTableEvent{ std::string(match[2]), std::string(match[1]) };
    }

    // new table
    if (std::regex_match(input, match, newTableRegex)) {
        return NewTableEvent{ std::string(match[1]) };
    }

    // delete
    if (std::regex_match(input, match, deleteRegex)) {
        return DeleteEvent{ CellAddress::fromString(match[1]) };
    }

    // reference
    if (std::regex_match(input, match, referenceRegex)) {
        return ReferenceEvent{
            CellAddress::fromString(match[1]),
            CellAddress::fromString(match[2])
        };
    }

    // insert
    if (std::regex_match(input, match, insertRegex)) {
        std::string raw = match[2];
        LiteralValue literal;

        if (raw == "TRUE" || raw == "FALSE") {
            literal.value = (raw == "TRUE");
        }
        else if (std::regex_match(raw, numberRegex)) {
            literal.value = std::stod(raw);
        }
        else {
            literal.value = raw;
        }

        return InsertEvent{ CellAddress::fromString(match[1]), literal };
    }

    // formula
    if (std::regex_match(input, match, formulaRegex)) {
        CellAddress address = CellAddress::fromString(match[1]);
        std::string funcName = match[2];
        std::string params = match[3];

        FormulaType type;
        if (funcName == "SUM") type = FormulaType::SUM;
        else if (funcName == "AVERAGE") type = FormulaType::AVERAGE;
        else if (funcName == "MIN") type = FormulaType::MIN;
        else if (funcName == "MAX") type = FormulaType::MAX;
        else if (funcName == "CONCAT") type = FormulaType::CONCAT;
        else if (funcName == "SUBSTR") type = FormulaType::SUBSTR;
        else if (funcName == "LEN") type = FormulaType::LEN;
        else if (funcName == "COUNT") type = FormulaType::COUNT;
        else throw std::invalid_argument("Unknown formula type: " + funcName);

        std::vector<FormulaParam> parsedParams;
        std::istringstream ss(params);
        std::string token;

        while (std::getline(ss, token, ',')) {
            token = trim(token);

            if (std::regex_match(token, numberRegex)) {
                parsedParams.push_back(LiteralValue{ std::stod(token) });
            }
            else if (token == "TRUE" || token == "FALSE") {
                parsedParams.push_back(LiteralValue{ token == "TRUE" });
            }
            else if (std::regex_match(token, rangeRegex)) {
                auto delim = token.find(":");
                parsedParams.push_back(AddressRange{
                    CellAddress::fromString(token.substr(0, delim)),
                    CellAddress::fromString(token.substr(delim + 1))
                    });
            }
            else if (std::regex_match(token, addressRegex)) {
                parsedParams.push_back(CellAddress::fromString(token));
            }
            else {
                parsedParams.push_back(LiteralValue{ token });
            }
        }

        return FormulaEvent{ address, type, parsedParams };
    }

    throw std::invalid_argument("Could not parse input: " + input);
}
