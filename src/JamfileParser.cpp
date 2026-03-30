#include "JamfileParser.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

JamfileData JamfileParser::parse(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return parseString(content);
}

JamfileData JamfileParser::parseString(const std::string& content) {
    JamfileData data;

    // Jamfile assignments can span multiple lines before the terminating ';'.
    // We accumulate lines into a buffer, and whenever we find a ';' we treat
    // everything from the last variable assignment keyword to that ';' as one
    // logical statement.

    std::string buffer;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        // Strip comments (but not inside quotes)
        bool inQuote = false;
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '"') inQuote = !inQuote;
            if (line[i] == '#' && !inQuote) {
                line = line.substr(0, i);
                break;
            }
        }

        buffer += " " + line;

        // Process all complete statements (terminated by ';') in the buffer
        size_t semiPos;
        while ((semiPos = buffer.find(';')) != std::string::npos) {
            std::string stmt = buffer.substr(0, semiPos);
            buffer = buffer.substr(semiPos + 1);
            parseLine(stmt, data);
        }
    }

    return data;
}

void JamfileParser::parseLine(const std::string& line, JamfileData& data) {
    // Find the '=' sign
    auto eqPos = line.find('=');
    if (eqPos == std::string::npos) return;

    // Check it's not '+=' or '?='... for now we only handle simple '='
    std::string lhs = line.substr(0, eqPos);
    std::string rhs = line.substr(eqPos + 1);

    // Trim whitespace from variable name
    auto ltrim = [](std::string& s) {
        s.erase(s.begin(),
                std::find_if(s.begin(), s.end(),
                             [](unsigned char c) { return !std::isspace(c); }));
    };
    auto rtrim = [](std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](unsigned char c) { return !std::isspace(c); })
                    .base(),
                s.end());
    };

    ltrim(lhs);
    rtrim(lhs);
    ltrim(rhs);
    rtrim(rhs);

    if (lhs.empty()) return;

    auto tokens = tokenize(rhs);

    auto joinTokens = [&tokens]() -> std::string {
        std::string result;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) result += " ";
            result += tokens[i];
        }
        return result;
    };

    if (lhs == "NAME") {
        data.name = joinTokens();
    } else if (lhs == "TYPE") {
        data.type = tokens.empty() ? "" : tokens[0];
    } else if (lhs == "APP_MIME_SIG") {
        data.appMimeSig = joinTokens();
    } else if (lhs == "SRCS") {
        data.srcs = tokens;
    } else if (lhs == "RSRCS") {
        data.rsrcs = tokens;
    } else if (lhs == "RDEFS") {
        data.rdefs = tokens;
    } else if (lhs == "LIBS") {
        data.libs = tokens;
    } else if (lhs == "LIBPATHS") {
        data.libPaths = tokens;
    } else if (lhs == "SYSTEM_INCLUDE_PATHS") {
        data.systemIncludePaths = tokens;
    } else if (lhs == "LOCAL_INCLUDE_PATHS") {
        data.localIncludePaths = tokens;
    } else if (lhs == "OPTIMIZE") {
        data.optimize = tokens.empty() ? "" : tokens[0];
    } else if (lhs == "LOCALES") {
        data.locales = tokens;
    } else if (lhs == "DEFINES") {
        data.defines = tokens;
    } else if (lhs == "WARNINGS") {
        data.warnings = tokens.empty() ? "" : tokens[0];
    } else if (lhs == "SYMBOLS") {
        data.symbols = tokens.empty() ? "" : tokens[0];
    } else if (lhs == "DEBUGGER") {
        data.debugger = tokens.empty() ? "" : tokens[0];
    } else if (lhs == "COMPILER_FLAGS") {
        data.compilerFlags = joinTokens();
    } else if (lhs == "LINKER_FLAGS") {
        data.linkerFlags = joinTokens();
    } else if (lhs == "DRIVER_PATH") {
        data.driverPath = joinTokens();
    }
}

std::vector<std::string> JamfileParser::tokenize(const std::string& valueStr) {
    std::vector<std::string> tokens;
    bool inQuote = false;
    std::string current;

    for (size_t i = 0; i < valueStr.size(); ++i) {
        char c = valueStr[i];
        if (c == '\\' && i + 1 < valueStr.size() && valueStr[i + 1] == '"') {
            // Backslash-escaped quote: preserve both characters as literals
            current += "\\\"";
            ++i;
        } else if (c == '"') {
            inQuote = !inQuote;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !inQuote) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}
