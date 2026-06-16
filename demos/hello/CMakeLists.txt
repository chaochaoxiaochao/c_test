add_library(hello_support
    hello_support.cpp
)
target_include_directories(hello_support PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_demo(hello
    hello_demo.cpp
)
target_link_libraries(demo_hello PRIVATE
    hello_support
)

add_simple_test(hello
    SOURCES
        tests/hello_simple_test.cpp
    INCLUDE_DIRS
        include
    LIBRARIES
        hello_support
)

add_catch2_test(hello
    SOURCES
        tests/hello_catch2_test.cpp
    INCLUDE_DIRS
        include
    LIBRARIES
        hello_support
)

add_benchmark(hello
    benchmarks/hello_benchmark.cpp
)
