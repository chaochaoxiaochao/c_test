cmake_minimum_required(VERSION 3.16)

project(c_test LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectOptions)

find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    find_package(Catch2 REQUIRED)
endif()
message(STATUS "Found Catch2 ${Catch2_VERSION}")

find_package(benchmark REQUIRED)

include(AddDemo)
include(AddSimpleTest)
include(AddCatch2Test)
include(AddBenchmark)

include(CTest)
enable_testing()

add_custom_target(run_tests)
add_custom_target(run_simple_tests)
add_custom_target(run_catch2_tests)
add_custom_target(run_benchmarks)

file(GLOB demo_dirs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/demos/*")
foreach(demo_dir IN LISTS demo_dirs)
    if(IS_DIRECTORY "${demo_dir}" AND EXISTS "${demo_dir}/CMakeLists.txt")
        add_subdirectory("${demo_dir}")
    endif()
endforeach()
