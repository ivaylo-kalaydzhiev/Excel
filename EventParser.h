#pragma once

#include <string>
#include "Event.h"

class EventParser {
public:
    static Event parse(const std::string& input);
};
