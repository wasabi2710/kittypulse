cmake_minimum_required(VERSION 3.30)
project(kittypulse)

if (WIN32) 

    add_executable(${PROJECT_NAME} src/win32/main.c)
    find_package(SDL3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

    file(GLOB DLLS "${CMAKE_SOURCE_DIR}/dlls/*.dll")

    add_custom_target(copy_dlls
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DLLS}
            "${CMAKE_BINARY_DIR}"
        COMMENT "Copying dlls to build directory"
    )

    add_dependencies(${PROJECT_NAME} copy_dlls)

endif()
