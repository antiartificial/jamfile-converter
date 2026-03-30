cmake_minimum_required(VERSION 3.16)
project(jamfile-converter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(jamfile-converter
    src/main.cpp
    src/JamfileParser.cpp
    src/CMakeGenerator.cpp
)

target_include_directories(jamfile-converter PRIVATE src)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    add_executable(jamfile-converter-tests
        tests/test_parser.cpp
        src/JamfileParser.cpp
        src/CMakeGenerator.cpp
    )
    target_include_directories(jamfile-converter-tests PRIVATE src)
    add_test(NAME jamfile-converter-tests COMMAND jamfile-converter-tests)
endif()
