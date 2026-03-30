BUILD_DIR := build

.PHONY: all build test clean run

all: build

build:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR)

test: build
	$(BUILD_DIR)/jamfile-converter-tests

clean:
	rm -rf $(BUILD_DIR)

run: build
	@echo "Usage: make run FILE=path/to/Jamfile [OUT=output/dir]"
	@if [ -z "$(FILE)" ]; then echo "Error: FILE is required"; exit 1; fi
	$(BUILD_DIR)/jamfile-converter $(FILE) $(OUT)
