# AGENTS.md

Guidance for coding agents working in this repository.

## Project

`c_test` is a C++17 CMake playground organized around self-contained demo directories. Each demo can define its own executable, simple tests, Catch2 tests, and Google Benchmark targets.

## Read First

- Build, run, test, and benchmark commands: `build.md`
- Human-facing overview and add-a-demo example: `README.md`

## Repository Layout

```text
cmake/                  Shared CMake helper functions
include/                Public headers, if a demo needs shared headers
src/                    Shared source files, if a demo needs shared code
demos/<name>/           Self-contained demo directory
demos/<name>/tests/     Simple and Catch2 tests for that demo
demos/<name>/benchmarks/ Google Benchmark sources for that demo
```

## CMake Conventions

- The root `CMakeLists.txt` owns global project setup, dependency discovery, aggregate targets, and automatic discovery of `demos/*/CMakeLists.txt`.
- Do not add one hard-coded `add_subdirectory(demos/<name>)` line per demo; creating `demos/<name>/CMakeLists.txt` is enough.
- Each `demos/<name>/CMakeLists.txt` owns that demo's executable, tests, and benchmarks.
- Use the shared helpers instead of writing ad-hoc targets:
  - `add_demo(<name> ...)`
  - `add_simple_test(<name> ...)`
  - `add_catch2_test(<name> ...)`
  - `add_benchmark(<name> ...)`
- For test-only dependencies, prefer the keyword form: `add_simple_test(<name> SOURCES ... INCLUDE_DIRS ... LIBRARIES ...)` and `add_catch2_test(<name> SOURCES ... INCLUDE_DIRS ... LIBRARIES ...)`.
- `${PROJECT_SOURCE_DIR}/include` is added to demo and test targets automatically; pass demo-local headers through `INCLUDE_DIRS` and shared support libraries through `LIBRARIES`.

## Target Naming

For demo name `<name>`, keep these stable target names:

```text
demo_<name>
simple_test_<name>
run_simple_<name>
catch2_test_<name>
run_catch2_<name>
bench_<name>
run_bench_<name>
```

Aggregate targets:

```text
run_tests
run_simple_tests
run_catch2_tests
run_benchmarks
```

## Dependencies

Dependencies are system-provided. Do not add `FetchContent` or network downloads to CMake unless the project direction changes.

Required packages:

- CMake
- C++17 compiler
- Catch2 CMake package; v3 preferred, v2 supported
- Google Benchmark CMake package

## Verification

After changing build logic or demo registration, run the relevant commands from `build.md`. The root `Makefile` is a wrapper around CMake, so these are valid from the repository root:

```bash
make all
make run_tests
```

Direct CMake commands are also valid:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If `cmake` or dependencies are unavailable in the current environment, state that explicitly and still run static checks against CMake files and target names.
