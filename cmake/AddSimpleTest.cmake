function(add_simple_test name)
    if(ARGC LESS 2)
        message(FATAL_ERROR "add_simple_test(<name> <sources...>) requires at least one source file")
    endif()

    set(target "simple_test_${name}")
    set(run_target "run_simple_${name}")
    set(test_name "simple_${name}")

    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )

    add_test(NAME ${test_name} COMMAND ${target})

    add_custom_target(${run_target}
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^${test_name}$"
        DEPENDS ${target}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        USES_TERMINAL
    )

    add_dependencies(run_simple_tests ${run_target})
    add_dependencies(run_tests ${run_target})
endfunction()
