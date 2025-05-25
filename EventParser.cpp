#include "EventParser.h"
#include "CellAddress.h"
#include <sstream>
#include <regex>
#include <stdexcept>

Event EventParser::parse(const std::string& input) {
    std::smatch match;

    // delete
    if (std::regex_match(input, match, std::regex(R"(([A-Z][0-9]+) delete)"))) {
        return DeleteEvent{ CellAddress::fromString(match[1]) };
    }

    // reference
    if (std::regex_match(input, match, std::regex(R"(([A-Z][0-9]+)=([A-Z][0-9]+))"))) {
        return ReferenceEvent{ CellAddress::fromString(match[1]), CellAddress::fromString(match[2]) };
    }

    // insert
    if (std::regex_match(input, match, std::regex(R"(([A-Z][0-9]+) insert (.+))"))) {
        std::string raw = match[2];
        
        LiteralValue literal;
        if (raw == "TRUE" || raw == "FALSE") literal.value = (raw == "TRUE");
        else if (std::regex_match(raw, std::regex(R"(\d+)"))) literal.value = std::stoi(raw);
        else literal.value = raw;
        
        return InsertEvent{ CellAddress::fromString(match[1]), literal };
    }

    // formula
    if (std::regex_match(input, match, std::regex(R"(([A-Z][0-9]+)=(\w+)\((.*)\))"))) {
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
        else throw std::invalid_argument("Unknown formula type");

        std::vector<FormulaParam> parsedParams;
        std::istringstream ss(params);
        std::string token;
        while (std::getline(ss, token, ',')) {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);

            // literal number
            if (std::regex_match(token, std::regex(R"(\d+)")))
                parsedParams.push_back(LiteralValue{ std::stoi(token) });
            // literal bool
            else if (token == "TRUE" || token == "FALSE")
                parsedParams.push_back(LiteralValue{ token == "TRUE" });
            // range
            else if (std::regex_match(token, std::regex(R"([A-Z][0-9]+:[A-Z][0-9]+)"))) {
                auto delim = token.find(":");
                parsedParams.push_back(AddressRange{
                    CellAddress::fromString(token.substr(0, delim)),
                    CellAddress::fromString(token.substr(delim + 1)) });
            }
            // reference address
            else if (std::regex_match(token, std::regex(R"([A-Z][0-9]+)"))) {
                parsedParams.push_back(CellAddress::fromString(token));
            }
            // literal string
            else {
                parsedParams.push_back(LiteralValue{ token });
            }
        }
    }

    throw std::invalid_argument("Could not parse input: " + input);
}
