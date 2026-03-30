# jamfile-converter

A tool that takes a Haiku [Jamfile-engine](https://github.com/haiku/jamfile-engine) `Jamfile` and spits out a `CMakeLists.txt`. Written in C++, built with CMake.

This came out of [VitruvianOS/Vitruvian#164](https://github.com/VitruvianOS/Vitruvian/issues/164).

## What it does

The Jamfile-engine is the standard build system for Haiku apps. You fill out a template with your project name, source files, libraries, etc., and Jam does the rest. This converter reads that template and generates an equivalent CMake project.

It handles all the standard variables:

- `NAME`, `TYPE` (APP, SHARED, STATIC, DRIVER)
- `SRCS`, `RSRCS`, `RDEFS`
- `LIBS`, `LIBPATHS`
- `SYSTEM_INCLUDE_PATHS`, `LOCAL_INCLUDE_PATHS`
- `OPTIMIZE`, `WARNINGS`, `SYMBOLS`, `DEBUGGER`
- `DEFINES`, `COMPILER_FLAGS`, `LINKER_FLAGS`
- `DRIVER_PATH` (generates an `install()` rule pointing to the right `/dev/` location)

A few things it handles that you might not expect:

- Quoted names with spaces (`NAME = "My Cool App" ;`) get properly quoted everywhere in the CMake output
- Library targets named `libfoo.so` or `libfoo.a` get the prefix/suffix stripped since CMake adds those itself
- Backslash-escaped quotes in `DEFINES` (e.g. `VERSION=\"1.0\"`) are preserved as-is

## Building

```bash
cmake -B build
cmake --build build
```

Requires CMake 3.16+ and a C++17-capable compiler. Works on macOS, Linux, Windows, and Haiku.

## Usage

```bash
# Print generated CMakeLists.txt to stdout
./build/jamfile-converter path/to/Jamfile

# Write CMakeLists.txt directly to a directory
./build/jamfile-converter path/to/Jamfile path/to/output/
```

## Running the tests

```bash
./build/jamfile-converter-tests
```

There are 15 unit tests covering parsing and CMake generation. They run in a second or two.

## How we tested it

The unit tests cover the obvious stuff: basic parsing, multiline values, comments, empty fields, all four target types, quoted names, debug flags, etc.

For more real-world testing we grabbed Makefiles from actual HaikuArchives apps (the Makefile-engine uses the same variable names as the Jamfile-engine, just different syntax) and converted them by hand to Jamfile format:

- **[ArtPaint](https://github.com/HaikuArchives/ArtPaint)** - a full painting app with 96 source files spread across multiple subdirectories, locales, `-Werror`
- **[Slayer](https://github.com/HaikuArchives/Slayer)** - a process manager, 13 source files, several Haiku-specific libs
- **[Weather](https://github.com/HaikuArchives/Weather)** - a weather app using private Haiku network headers

All three converted without issues, source file counts match exactly.

The test Jamfiles live in `tests/test_data/` if you want to look at them or run the converter against them manually.
