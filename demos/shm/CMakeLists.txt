
# add_library(own_shm
#     hello_support.cpp
# )

# target_include_directories(hello_support PUBLIC
#     "${CMAKE_CURRENT_SOURCE_DIR}/include"
# )

add_catch2_test(test_shm
    SOURCES
        tests/shm_catch2_test.cpp
    INCLUDE_DIRS
        include
)
