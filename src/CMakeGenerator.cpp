#include "CMakeGenerator.h"

#include <algorithm>
#include <sstream>

static std::string mapOptimizeLevel(const std::string& opt) {
    if (opt == "NONE") return "0";
    if (opt == "SOME") return "2";
    if (opt == "FULL") return "3";
    return "";
}

static bool hasSpaces(const std::string& s) {
    return s.find(' ') != std::string::npos;
}

static std::string quote(const std::string& s) {
    if (hasSpaces(s)) return "\"" + s + "\"";
    return s;
}

static std::string stripLibAffixes(const std::string& name) {
    std::string result = name;
    if (result.size() > 3 && result.substr(0, 3) == "lib") {
        result = result.substr(3);
    }
    if (result.size() > 3 && result.substr(result.size() - 3) == ".so") {
        result = result.substr(0, result.size() - 3);
    } else if (result.size() > 2 && result.substr(result.size() - 2) == ".a") {
        result = result.substr(0, result.size() - 2);
    }
    return result;
}

std::string CMakeGenerator::generate(const JamfileData& data) {
    std::ostringstream out;

    std::string projectName = data.name.empty() ? "MyProject" : data.name;

    // For library targets, strip lib prefix and .so/.a suffix since CMake
    // handles these automatically.
    std::string targetName = projectName;
    if (data.type == "SHARED" || data.type == "STATIC") {
        targetName = stripLibAffixes(projectName);
    }

    out << "cmake_minimum_required(VERSION 3.16)\n";
    out << "project(" << quote(projectName) << " LANGUAGES CXX)\n\n";
    out << "set(CMAKE_CXX_STANDARD 17)\n";
    out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

    // Sources
    if (!data.srcs.empty()) {
        out << "set(SOURCES\n";
        for (const auto& src : data.srcs) {
            out << "    " << src << "\n";
        }
        out << ")\n\n";
    }

    std::string tgt = quote(targetName);

    // Target
    if (data.type == "APP") {
        out << "add_executable(" << tgt;
        if (!data.srcs.empty()) {
            out << " ${SOURCES}";
        }
        out << ")\n\n";
    } else if (data.type == "SHARED") {
        out << "add_library(" << tgt << " SHARED";
        if (!data.srcs.empty()) {
            out << " ${SOURCES}";
        }
        out << ")\n\n";
    } else if (data.type == "STATIC") {
        out << "add_library(" << tgt << " STATIC";
        if (!data.srcs.empty()) {
            out << " ${SOURCES}";
        }
        out << ")\n\n";
    } else if (data.type == "DRIVER") {
        out << "# Haiku kernel driver -- mapped to MODULE library\n";
        out << "add_library(" << tgt << " MODULE";
        if (!data.srcs.empty()) {
            out << " ${SOURCES}";
        }
        out << ")\n\n";
    } else {
        out << "add_executable(" << tgt;
        if (!data.srcs.empty()) {
            out << " ${SOURCES}";
        }
        out << ")\n\n";
    }

    // Include paths
    bool hasIncludes =
        !data.systemIncludePaths.empty() || !data.localIncludePaths.empty();
    if (hasIncludes) {
        out << "target_include_directories(" << tgt << "\n";
        if (!data.localIncludePaths.empty()) {
            out << "    PRIVATE\n";
            for (const auto& p : data.localIncludePaths) {
                out << "        " << p << "\n";
            }
        }
        if (!data.systemIncludePaths.empty()) {
            out << "    SYSTEM PRIVATE\n";
            for (const auto& p : data.systemIncludePaths) {
                out << "        " << p << "\n";
            }
        }
        out << ")\n\n";
    }

    // Library search paths
    if (!data.libPaths.empty()) {
        for (const auto& lp : data.libPaths) {
            out << "link_directories(" << lp << ")\n";
        }
        out << "\n";
    }

    // Link libraries
    if (!data.libs.empty()) {
        out << "target_link_libraries(" << tgt << " PRIVATE\n";
        for (const auto& lib : data.libs) {
            out << "    " << lib << "\n";
        }
        out << ")\n\n";
    }

    // Defines
    if (!data.defines.empty()) {
        out << "target_compile_definitions(" << tgt << " PRIVATE\n";
        for (const auto& def : data.defines) {
            out << "    " << def << "\n";
        }
        out << ")\n\n";
    }

    // Optimization
    std::string optLevel = mapOptimizeLevel(data.optimize);
    if (!optLevel.empty()) {
        out << "target_compile_options(" << tgt << " PRIVATE -O"
            << optLevel << ")\n\n";
    }

    // Warnings
    if (data.warnings == "ALL") {
        out << "target_compile_options(" << tgt
            << " PRIVATE -Wall -Wextra)\n\n";
    } else if (data.warnings == "NONE") {
        out << "target_compile_options(" << tgt << " PRIVATE -w)\n\n";
    }

    // Debug symbols
    if (data.symbols == "TRUE") {
        out << "target_compile_options(" << tgt << " PRIVATE -g)\n\n";
    }

    // Debugger (implies debug build)
    if (data.debugger == "TRUE") {
        out << "target_compile_options(" << tgt
            << " PRIVATE -g -O0)\n\n";
    }

    // Extra compiler flags
    if (!data.compilerFlags.empty()) {
        out << "target_compile_options(" << tgt << " PRIVATE "
            << data.compilerFlags << ")\n\n";
    }

    // Extra linker flags
    if (!data.linkerFlags.empty()) {
        out << "target_link_options(" << tgt << " PRIVATE "
            << data.linkerFlags << ")\n\n";
    }

    // Driver path as install hint
    if (!data.driverPath.empty()) {
        out << "# Haiku driver install path: /dev/" << data.driverPath << "\n";
        out << "install(TARGETS " << tgt << " DESTINATION add-ons/kernel/drivers/dev/"
            << data.driverPath << ")\n\n";
    }

    return out.str();
}
