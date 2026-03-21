.PHONY: default clang-release clang-debug emsdk emrun web-release web-debug build format tidy test clean

MAKE := $(MAKE)
MAKEFLAGS += --no-print-directory
# Adjust parallel build jobs based on your available cores.
# Try linux environment first then applex86 or M1, then give up and just do one
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo "-j$$(nproc)") || (command -v sysctl -n hw.ncpu > /dev/null 2>&1 && echo "-j$$(sysctl -n hw.ncpu)") || echo "")

default: build

emsdk:
	cd deps/emsdk && ./emsdk install latest && ./emsdk activate latest

emrun:
	@if [ -x "build/debug/bin" ]; then                                       \
		./deps/emsdk/upstream/emscripten/emrun ./build/debug/bin/index.html; \
	elif [ -x "build/bin/" ]; then                                           \
		./deps/emsdk/upstream/emscripten/emrun ./build/bin/index.html;       \
	else                                                                     \
		echo "No index.html found.";                                         \
		exit 1;                                                              \
	fi

clang-release:
	cmake --preset=clang-release
	cmake --build build/ $(JOBS)

clang-debug:
	cmake --preset=clang-debug
	cmake --build build/ $(JOBS)

web-release:
	cmake --preset=web-release
	cmake --build build/ $(JOBS) --target pokemon_gui

web-debug:
	cmake --preset=web-debug
	cmake --build build/ $(JOBS) --target pokemon_gui

build:
	cmake --build build/ $(JOBS)

format:
	cmake --build build/ --target format

tidy:
	cmake --build build/ --target tidy $(JOBS)

test:
	cmake --build build/ $(JOBS)
	@if [ -x "build/debug/bin/tests" ]; then       \
		./build/debug/bin/tests --gtest_color=yes; \
	elif [ -x "build/bin/tests" ]; then            \
		./build/bin/tests --gtest_color=yes;       \
	else                                           \
		echo "No google tests found";              \
		exit 1;                                    \
	fi
	@echo "RAN TESTS"


clean:
	rm -rf build/
