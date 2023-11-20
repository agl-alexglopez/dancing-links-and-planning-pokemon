.PHONY: default gcc-release gcc-debug clang-release clang-debug debug-tests release-tests build format tidy clean

MAKE := $(MAKE)
BUILD_DIR := build/
# Adjust parallel build jobs based on your available cores.
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo -n "-j$$(nproc)") || echo -n "")

default: build

gcc-release:
	@cmake --preset=gcc-release
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)

gcc-debug:
	@cmake --preset=gcc-debug
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)

clang-release:
	@cmake --preset=clang-release
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)

clang-debug:
	@cmake --preset=clang-debug
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)

build:
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)

format:
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) format

tidy:
	@$(MAKE) --no-print-directory -C $(BUILD_DIR) tidy

debug-tests:
	$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)
	$(BUILD_DIR)debug/tests

release-tests:
	$(MAKE) --no-print-directory -C $(BUILD_DIR) $(JOBS)
	$(BUILD_DIR)release/tests

clean:
	rm -rf build/
