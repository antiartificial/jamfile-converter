#include "CMakeGenerator.h"
#include "JamfileParser.h"

#include <fstream>
#include <iostream>
#include <string>

static void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " <Jamfile> [output-dir]\n"
              << "\n"
              << "Converts a Haiku Jamfile-engine Jamfile to a CMakeLists.txt.\n"
              << "If output-dir is given, writes CMakeLists.txt there; otherwise\n"
              << "prints to stdout.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputDir;
    if (argc >= 3) {
        outputDir = argv[2];
    }

    JamfileParser parser;
    JamfileData data;

    try {
        data = parser.parse(inputPath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    CMakeGenerator generator;
    std::string cmake = generator.generate(data);

    if (outputDir.empty()) {
        std::cout << cmake;
    } else {
        std::string outPath = outputDir;
        if (outPath.back() != '/') outPath += '/';
        outPath += "CMakeLists.txt";

        std::ofstream outFile(outPath);
        if (!outFile.is_open()) {
            std::cerr << "Error: cannot write to " << outPath << "\n";
            return 1;
        }
        outFile << cmake;
        std::cout << "Wrote " << outPath << "\n";
    }

    return 0;
}
