.PHONY: default rel deb build format tidy clean

MAKE := $(MAKE)
MAKEFLAGS += --no-print-directory
# Adjust parallel build jobs based on your available cores.
# Try linux environment first then applex86 or M1, then give up and just do one
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo "-j$$(nproc)") || (command -v sysctl -n hw.ncpu > /dev/null 2>&1 && echo "-j$$(sysctl -n hw.ncpu)") || echo "")

default: build

rel:
	cmake --preset=rel
	cmake --build build/ $(JOBS)

deb:
	cmake --preset=deb
	cmake --build build/ $(JOBS)

build:
	cmake --build build/ $(JOBS)

format:
	cmake --build build/ --target format

tidy:
	cmake --build build/ --target tidy $(JOBS)

clean:
	rm -rf build/
