.PHONY: configure all clean distclean help

Makefile: ;

BUILD_DIR ?= build
CMAKE ?= cmake

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

all: configure
	$(CMAKE) --build $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Removed $(BUILD_DIR)/"

distclean: clean

help:
	@echo "Usage:"
	@echo "  make all                 Configure and build everything"
	@echo "  make <target>            Configure and build a CMake target"
	@echo "  make run_tests           Run all tests"
	@echo "  make run_simple_hello    Run one simple test"
	@echo "  make run_catch2_hello    Run one Catch2 test"
	@echo "  make run_bench_hello     Run one benchmark"
	@echo "  make clean               Remove the CMake build tree"

%: configure
	$(CMAKE) --build $(BUILD_DIR) --target $@
