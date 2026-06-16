function(add_catch2_test name)
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCES INCLUDE_DIRS LIBRARIES)
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(ARG_SOURCES)
        set(test_sources ${ARG_SOURCES})
    else()
        set(test_sources ${ARG_UNPARSED_ARGUMENTS})
    endif()

    if(NOT test_sources)
        message(FATAL_ERROR "add_catch2_test(<name> <sources...>) or add_catch2_test(<name> SOURCES <sources...>) requires at least one source file")
    endif()

    set(target "catch2_test_${name}")
    set(run_target "run_catch2_${name}")
    set(test_name "catch2_${name}")

    add_executable(${target} ${test_sources})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )

    if(ARG_INCLUDE_DIRS)
        target_include_directories(${target} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()

    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    if(ARG_LIBRARIES)
        target_link_libraries(${target} PRIVATE ${ARG_LIBRARIES})
    endif()

    add_test(NAME ${test_name} COMMAND ${target})

    add_custom_target(${run_target}
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^${test_name}$"
        DEPENDS ${target}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        USES_TERMINAL
    )

    add_dependencies(run_catch2_tests ${run_target})
    add_dependencies(run_tests ${run_target})
endfunction()
