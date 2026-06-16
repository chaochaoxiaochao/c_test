function(add_benchmark name)
    if(ARGC LESS 2)
        message(FATAL_ERROR "add_benchmark(<name> <sources...>) requires at least one source file")
    endif()

    set(target "bench_${name}")
    set(run_target "run_bench_${name}")

    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )
    target_link_libraries(${target} PRIVATE benchmark::benchmark)

    add_custom_target(${run_target}
        COMMAND $<TARGET_FILE:${target}>
        DEPENDS ${target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL
    )

    add_dependencies(run_benchmarks ${run_target})
endfunction()
