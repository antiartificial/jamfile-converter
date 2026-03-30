#pragma once

#include "JamfileParser.h"

#include <string>

class CMakeGenerator {
public:
    std::string generate(const JamfileData& data);
};
