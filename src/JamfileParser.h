#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct JamfileData {
    std::string name;
    std::string type; // APP, SHARED, STATIC, DRIVER
    std::string appMimeSig;
    std::vector<std::string> srcs;
    std::vector<std::string> rsrcs;
    std::vector<std::string> rdefs;
    std::vector<std::string> libs;
    std::vector<std::string> libPaths;
    std::vector<std::string> systemIncludePaths;
    std::vector<std::string> localIncludePaths;
    std::string optimize; // NONE, SOME, FULL
    std::vector<std::string> locales;
    std::vector<std::string> defines;
    std::string warnings; // NONE, ALL
    std::string symbols;  // TRUE or empty
    std::string debugger; // TRUE or empty
    std::string compilerFlags;
    std::string linkerFlags;
    std::string driverPath;
};

class JamfileParser {
public:
    JamfileData parse(const std::string& filePath);
    JamfileData parseString(const std::string& content);

private:
    void parseLine(const std::string& line, JamfileData& data);
    std::vector<std::string> tokenize(const std::string& valueStr);
};
