#include "CMakeGenerator.h"
#include "JamfileParser.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

static void testBasicParse() {
    std::string jamfile =
        "NAME = HelloWorld ;\n"
        "TYPE = APP ;\n"
        "SRCS = main.cpp util.cpp ;\n"
        "LIBS = be tracker ;\n"
        "SYSTEM_INCLUDE_PATHS = /boot/system/develop/headers ;\n"
        "LOCAL_INCLUDE_PATHS = src include ;\n"
        "OPTIMIZE = FULL ;\n"
        "WARNINGS = ALL ;\n"
        "DEFINES = DEBUG=1 HAIKU_TARGET ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    assert(data.name == "HelloWorld");
    assert(data.type == "APP");
    assert(data.srcs.size() == 2);
    assert(data.srcs[0] == "main.cpp");
    assert(data.srcs[1] == "util.cpp");
    assert(data.libs.size() == 2);
    assert(data.libs[0] == "be");
    assert(data.libs[1] == "tracker");
    assert(data.systemIncludePaths.size() == 1);
    assert(data.localIncludePaths.size() == 2);
    assert(data.optimize == "FULL");
    assert(data.warnings == "ALL");
    assert(data.defines.size() == 2);
    assert(data.defines[0] == "DEBUG=1");
    assert(data.defines[1] == "HAIKU_TARGET");

    std::cout << "  testBasicParse: PASSED\n";
}

static void testQuotedName() {
    std::string jamfile =
        "NAME = \"My Cool App\" ;\n"
        "TYPE = APP ;\n"
        "SRCS = main.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    assert(data.name == "My Cool App");
    assert(data.type == "APP");

    std::cout << "  testQuotedName: PASSED\n";
}

static void testSharedLibrary() {
    std::string jamfile =
        "NAME = mylib ;\n"
        "TYPE = SHARED ;\n"
        "SRCS = lib.cpp helpers.cpp ;\n"
        "LIBS = be ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("add_library(mylib SHARED") != std::string::npos);
    assert(cmake.find("${SOURCES}") != std::string::npos);
    assert(cmake.find("target_link_libraries(mylib PRIVATE") != std::string::npos);

    std::cout << "  testSharedLibrary: PASSED\n";
}

static void testStaticLibrary() {
    std::string jamfile =
        "NAME = mystaticlib ;\n"
        "TYPE = STATIC ;\n"
        "SRCS = core.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("add_library(mystaticlib STATIC") != std::string::npos);

    std::cout << "  testStaticLibrary: PASSED\n";
}

static void testDriver() {
    std::string jamfile =
        "NAME = usb_webcam ;\n"
        "TYPE = DRIVER ;\n"
        "SRCS = driver.cpp ;\n"
        "DRIVER_PATH = video/usb ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("add_library(usb_webcam MODULE") != std::string::npos);
    assert(cmake.find("add-ons/kernel/drivers/dev/video/usb") != std::string::npos);

    std::cout << "  testDriver: PASSED\n";
}

static void testComments() {
    std::string jamfile =
        "# This is a comment\n"
        "NAME = TestApp ; # inline comment\n"
        "TYPE = APP ;\n"
        "# SRCS = ignored.cpp ;\n"
        "SRCS = real.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    assert(data.name == "TestApp");
    assert(data.srcs.size() == 1);
    assert(data.srcs[0] == "real.cpp");

    std::cout << "  testComments: PASSED\n";
}

static void testMultilineValues() {
    std::string jamfile =
        "NAME = MultiLine ;\n"
        "TYPE = APP ;\n"
        "SRCS = main.cpp\n"
        "    utils.cpp\n"
        "    helpers.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    assert(data.srcs.size() == 3);
    assert(data.srcs[0] == "main.cpp");
    assert(data.srcs[1] == "utils.cpp");
    assert(data.srcs[2] == "helpers.cpp");

    std::cout << "  testMultilineValues: PASSED\n";
}

static void testEmptyValues() {
    std::string jamfile =
        "NAME = EmptyApp ;\n"
        "TYPE = APP ;\n"
        "SRCS = ;\n"
        "LIBS = ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    assert(data.name == "EmptyApp");
    assert(data.srcs.empty());
    assert(data.libs.empty());

    std::cout << "  testEmptyValues: PASSED\n";
}

static void testDebuggerFlags() {
    std::string jamfile =
        "NAME = DebugApp ;\n"
        "TYPE = APP ;\n"
        "SRCS = main.cpp ;\n"
        "SYMBOLS = TRUE ;\n"
        "DEBUGGER = TRUE ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("-g -O0") != std::string::npos);
    assert(cmake.find("target_compile_options(DebugApp PRIVATE -g)") != std::string::npos);

    std::cout << "  testDebuggerFlags: PASSED\n";
}

static void testFullGeneration() {
    std::string jamfile =
        "NAME = HelloWorld ;\n"
        "TYPE = APP ;\n"
        "APP_MIME_SIG = x-vnd.Test-HelloWorld ;\n"
        "SRCS = main.cpp ;\n"
        "LIBS = be locale localestub ;\n"
        "SYSTEM_INCLUDE_PATHS = /boot/system/develop/headers ;\n"
        "LOCAL_INCLUDE_PATHS = src ;\n"
        "OPTIMIZE = SOME ;\n"
        "WARNINGS = ALL ;\n"
        "DEFINES = HAIKU_TARGET ;\n"
        "COMPILER_FLAGS = -fPIC ;\n"
        "LINKER_FLAGS = -Wl,--no-undefined ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("project(HelloWorld") != std::string::npos);
    assert(cmake.find("add_executable(HelloWorld") != std::string::npos);
    assert(cmake.find("target_link_libraries(HelloWorld PRIVATE") != std::string::npos);
    assert(cmake.find("be") != std::string::npos);
    assert(cmake.find("-O2") != std::string::npos);
    assert(cmake.find("-Wall -Wextra") != std::string::npos);
    assert(cmake.find("-fPIC") != std::string::npos);
    assert(cmake.find("-Wl,--no-undefined") != std::string::npos);
    assert(cmake.find("HAIKU_TARGET") != std::string::npos);

    std::cout << "  testFullGeneration: PASSED\n";
}

static void testQuotedNameCMakeOutput() {
    std::string jamfile =
        "NAME = \"My Cool App\" ;\n"
        "TYPE = APP ;\n"
        "SRCS = App.cpp ;\n"
        "LIBS = be ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("project(\"My Cool App\"") != std::string::npos);
    assert(cmake.find("add_executable(\"My Cool App\"") != std::string::npos);
    assert(cmake.find("target_link_libraries(\"My Cool App\" PRIVATE") != std::string::npos);

    std::cout << "  testQuotedNameCMakeOutput: PASSED\n";
}

static void testLibraryNameStripping() {
    std::string jamfile =
        "NAME = libmyaddon.so ;\n"
        "TYPE = SHARED ;\n"
        "SRCS = addon.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("project(libmyaddon.so") != std::string::npos);
    assert(cmake.find("add_library(myaddon SHARED") != std::string::npos);

    std::cout << "  testLibraryNameStripping: PASSED\n";
}

static void testStaticLibNameStripping() {
    std::string jamfile =
        "NAME = libutil.a ;\n"
        "TYPE = STATIC ;\n"
        "SRCS = util.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("add_library(util STATIC") != std::string::npos);

    std::cout << "  testStaticLibNameStripping: PASSED\n";
}

static void testEscapedQuotesInDefines() {
    std::string jamfile =
        "NAME = TestApp ;\n"
        "TYPE = APP ;\n"
        "SRCS = main.cpp ;\n"
        "DEFINES = VERSION=\\\"3.10\\\" HAVE_SSL=1 ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    assert(data.defines.size() == 2);
    assert(data.defines[0] == "VERSION=\\\"3.10\\\"");
    assert(data.defines[1] == "HAVE_SSL=1");

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);
    assert(cmake.find("VERSION=\\\"3.10\\\"") != std::string::npos);

    std::cout << "  testEscapedQuotesInDefines: PASSED\n";
}

static void testNoLibPrefixStrippingForApp() {
    std::string jamfile =
        "NAME = libfoo ;\n"
        "TYPE = APP ;\n"
        "SRCS = main.cpp ;\n";

    JamfileParser parser;
    JamfileData data = parser.parseString(jamfile);

    CMakeGenerator gen;
    std::string cmake = gen.generate(data);

    assert(cmake.find("add_executable(libfoo") != std::string::npos);

    std::cout << "  testNoLibPrefixStrippingForApp: PASSED\n";
}

int main() {
    std::cout << "Running Jamfile converter tests...\n";

    testBasicParse();
    testQuotedName();
    testSharedLibrary();
    testStaticLibrary();
    testDriver();
    testComments();
    testMultilineValues();
    testEmptyValues();
    testDebuggerFlags();
    testFullGeneration();
    testQuotedNameCMakeOutput();
    testLibraryNameStripping();
    testStaticLibNameStripping();
    testEscapedQuotesInDefines();
    testNoLibPrefixStrippingForApp();

    std::cout << "All tests passed!\n";
    return 0;
}
