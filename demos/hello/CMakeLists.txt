add_demo(hello
    hello_demo.cpp
)

add_simple_test(hello
    tests/hello_simple_test.cpp
)

add_catch2_test(hello
    tests/hello_catch2_test.cpp
)

add_benchmark(hello
    benchmarks/hello_benchmark.cpp
)
