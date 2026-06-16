# Build and Run

This project is a C++17 CMake playground named `c_test`. It uses system-provided Catch2 and Google Benchmark.

## Dependencies

Install CMake, a C++17 compiler, Catch2, and Google Benchmark.

Ubuntu/Debian package names commonly look like:

```bash
sudo apt update
sudo apt install cmake g++ catch2 libbenchmark-dev
```

The build prefers Catch2 v3 when available and falls back to Catch2 v2. Ubuntu 22.04 commonly provides Catch2 2.13.8 via `catch2`, which is supported.

## Quick Root Make Commands

The repository root has a small `Makefile` wrapper. These commands configure `build/` automatically and then forward to CMake:

```bash
make all
make run_tests
make run_simple_hello
make run_catch2_hello
make run_bench_hello
make clean
```

`make clean` removes the whole `build/` directory. `make <target>` from the repository root is equivalent to:

```bash
cmake -S . -B build
cmake --build build --target <target>
```

## Configure

Run from the project root:

```bash
cmake -S . -B build
```

## Build Everything

```bash
cmake --build build
```

With Makefile generators, you can also run from `build/`:

```bash
make all
```

## Run Demo Executables

Build the hello demo:

```bash
cmake --build build --target demo_hello
```

Run it directly:

```bash
./build/demos/hello/demo_hello
```

Expected output:

```text
hello demo
```

## Run Tests

Run all registered tests through the aggregate target:

```bash
cmake --build build --target run_tests
```

Run all tests through CTest:

```bash
ctest --test-dir build --output-on-failure
```

Run one simple test:

```bash
cmake --build build --target run_simple_hello
```

Run one Catch2 test:

```bash
cmake --build build --target run_catch2_hello
```

Filter tests by name:

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

## Makefile Shortcuts

From the repository root, use the wrapper Makefile:

```bash
make all
make demo_hello
make run_tests
make run_simple_hello
make run_catch2_hello
make run_bench_hello
make clean
```

If CMake generated Makefiles, the same target names also work from `build/`. Prefer root `make clean` when you want a quiet full clean because it removes `build/` directly.

## Adding a New Demo

Create a self-contained directory:

```text
demos/<name>/
|-- CMakeLists.txt
|-- <name>_demo.cpp
|-- tests/
|   |-- <name>_simple_test.cpp
|   `-- <name>_catch2_test.cpp
`-- benchmarks/
    `-- <name>_benchmark.cpp
```

The root `CMakeLists.txt` automatically discovers `demos/*/CMakeLists.txt`, so do not manually add `add_subdirectory(demos/<name>)` for each demo.

Inside `demos/<name>/CMakeLists.txt`, use:

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
