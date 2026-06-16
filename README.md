# c_test

Small C++17 playground organized around self-contained demo directories. Each demo can define its own executable, simple tests, Catch2 tests, and Google Benchmark targets.

Build, run, test, and benchmark commands live in [`build.md`](build.md). Agent-specific repository guidance lives in [`AGENTS.md`](AGENTS.md).

## Dependencies

Install CMake, a C++17 compiler, Catch2, and Google Benchmark from the system package manager or from your platform's standard package source.

On Ubuntu, package names commonly look like this:

```bash
sudo apt install cmake g++ catch2 libbenchmark-dev
```

The build prefers Catch2 v3 when available and falls back to Catch2 v2. Ubuntu 22.04 commonly provides Catch2 2.13.8 via `catch2`, which is supported.

## Configure and Build

Use the root Makefile wrapper:

```bash
make all
make clean
```

`make clean` removes the whole `build/` directory.

Or call CMake directly:

```bash
cmake -S . -B build
cmake --build build
```

With Makefile generators, `make all` also works from `build/`.

## Run Tests

Run all tests:

```bash
cmake --build build --target run_tests
ctest --test-dir build --output-on-failure
```

Run one demo's simple test:

```bash
cmake --build build --target run_simple_hello
```

Run one demo's Catch2 test:

```bash
cmake --build build --target run_catch2_hello
```

Filter with CTest:

```bash
ctest --test-dir build -R hello --output-on-failure
```

## Run Benchmarks

Run all benchmarks:

```bash
cmake --build build --target run_benchmarks
```

Run one benchmark:

```bash
cmake --build build --target run_bench_hello
```

## Add a New Demo

Create a directory under `demos/<name>/`:

```text
demos/<name>/
├── CMakeLists.txt
├── <name>_demo.cpp
├── tests/
│   ├── <name>_simple_test.cpp
│   └── <name>_catch2_test.cpp
└── benchmarks/
    └── <name>_benchmark.cpp
```

The root `CMakeLists.txt` automatically discovers `demos/*/CMakeLists.txt`, so no per-demo root edit is needed.

Inside `demos/<name>/CMakeLists.txt`, use the shared helpers:

```cmake
add_demo(<name>
    <name>_demo.cpp
)

add_simple_test(<name>
    tests/<name>_simple_test.cpp
)

add_catch2_test(<name>
    tests/<name>_catch2_test.cpp
)

add_benchmark(<name>
    benchmarks/<name>_benchmark.cpp
)
```

When a test needs extra include directories or libraries, use the keyword form:

```cmake
add_simple_test(<name>
    SOURCES
        tests/<name>_simple_test.cpp
    INCLUDE_DIRS
        include
    LIBRARIES
        <name>_support
        Threads::Threads
)

add_catch2_test(<name>
    SOURCES
        tests/<name>_catch2_test.cpp
    INCLUDE_DIRS
        include
    LIBRARIES
        <name>_support
)
```

`${PROJECT_SOURCE_DIR}/include` is added automatically to every demo and test target. Put demo-private headers under `demos/<name>/include` and pass that path through `INCLUDE_DIRS`; put repeated logic in a small support library and pass it through `LIBRARIES`.

This creates stable targets:

```text
demo_<name>
simple_test_<name>
run_simple_<name>
catch2_test_<name>
run_catch2_<name>
bench_<name>
run_bench_<name>
```
