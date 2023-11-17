.PHONY: default release r debug d build format tidy clean

MAKE := $(MAKE)
# Adjust parallel build jobs based on your available cores.
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && nproc) || echo 1)

default: build

release:
	@cmake --preset=gcc-release
	@$(MAKE) --no-print-directory -C build/release -j$(JOBS)

r:
	@$(MAKE) --no-print-directory -C build/release -j$(JOBS)

debug:
	@cmake --preset=gcc-debug
	@$(MAKE) --no-print-directory -C build/debug -j$(JOBS)

d:
	@$(MAKE) --no-print-directory -C build/release -j$(JOBS)

build:
	@if [ -d build/debug ]; then \
		$(MAKE) --no-print-directory -C build/debug -j$(JOBS) || true; \
	fi
	@if [ -d build/release ]; then \
		$(MAKE) --no-print-directory -C build/release -j$(JOBS) || true; \
	fi

format:
	@$(MAKE) --no-print-directory -C build/ format

tidy:
	@$(MAKE) --no-print-directory -C build/ tidy

clean:
	rm -rf build/
