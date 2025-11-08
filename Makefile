.PHONY: default rel deb emsdk emrun webrel build format tidy dtest rtest clean

MAKE := $(MAKE)
MAKEFLAGS += --no-print-directory
# Adjust parallel build jobs based on your available cores.
# Try linux environment first then applex86 or M1, then give up and just do one
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo "-j$$(nproc)") || (command -v sysctl -n hw.ncpu > /dev/null 2>&1 && echo "-j$$(sysctl -n hw.ncpu)") || echo "")

default: build

emsdk:
	cd deps/emsdk && ./emsdk install latest && ./emsdk activate latest

emrun:
	./deps/emsdk/upstream/emscripten/emrun ./build/bin/pokemon_gui.html

rel:
	cmake --preset=rel
	cmake --build build/ $(JOBS)

deb:
	cmake --preset=deb
	cmake --build build/ $(JOBS)

webrel:
	cmake --preset=webrel
	cmake --build build/ $(JOBS) --target pokemon_gui

build:
	cmake --build build/ $(JOBS)

format:
	cmake --build build/ --target format

tidy:
	cmake --build build/ --target tidy $(JOBS)

dtest:
	cmake --build build/ $(JOBS)
	./build/debug/bin/tests --gtest_color=yes

rtest:
	cmake --build build/ $(JOBS)
	./build/bin/tests --gtest_color=yes

clean:
	rm -rf build/
