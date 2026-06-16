function(add_demo name)
    if(ARGC LESS 2)
        message(FATAL_ERROR "add_demo(<name> <sources...>) requires at least one source file")
    endif()

    set(target "demo_${name}")
    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )
endfunction()
