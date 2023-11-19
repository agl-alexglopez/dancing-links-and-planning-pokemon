.PHONY: default gcc-release gcc-debug clang-release clang-debug build format tidy clean

MAKE := $(MAKE)
# Adjust parallel build jobs based on your available cores.
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo -n "-j$$(nproc)") || echo -n "")

default: build

gcc-release:
	@cmake --preset=gcc-release
	@$(MAKE) --no-print-directory -C build/ $(JOBS)

gcc-debug:
	@cmake --preset=gcc-debug
	@$(MAKE) --no-print-directory -C build/ $(JOBS)

clang-release:
	@cmake --preset=clang-release
	@$(MAKE) --no-print-directory -C build/ $(JOBS)

clang-debug:
	@cmake --preset=clang-debug
	@$(MAKE) --no-print-directory -C build/ $(JOBS)

build:
	@$(MAKE) --no-print-directory -C build/ $(JOBS)

format:
	@$(MAKE) --no-print-directory -C build/ format

tidy:
	@$(MAKE) --no-print-directory -C build/ tidy

clean:
	rm -rf build/
