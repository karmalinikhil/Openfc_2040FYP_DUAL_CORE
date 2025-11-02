# OpenFC2040 Makefile
# Simple wrapper around build scripts

.PHONY: all build flash clean help setup

all: build

help:
	@echo "OpenFC2040 Firmware Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  build    - Build the firmware"
	@echo "  flash    - Flash firmware to board"
	@echo "  clean    - Clean build directory"
	@echo "  setup    - Initialize submodules"
	@echo "  help     - Show this help"

setup:
	@echo "Initializing Pico SDK submodules..."
	git submodule update --init --recursive

build:
	@./scripts/build.sh

flash:
	@./scripts/flash.sh

clean:
	@echo "Cleaning build directory..."
	rm -rf build/

# Default target
.DEFAULT_GOAL := help
