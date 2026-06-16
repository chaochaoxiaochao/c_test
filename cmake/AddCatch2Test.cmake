function(add_catch2_test name)
    if(ARGC LESS 2)
        message(FATAL_ERROR "add_catch2_test(<name> <sources...>) requires at least one source file")
    endif()

    set(target "catch2_test_${name}")
    set(run_target "run_catch2_${name}")
    set(test_name "catch2_${name}")

    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)

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
