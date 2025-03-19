cmake_minimum_required(VERSION 3.25)
project(kittypulse)

find_package(SDL3 REQUIRED)

if (WIN32)

    add_executable(${PROJECT_NAME} src/win32/main.c)
    find_package(SDL3_image REQUIRED)

    # Ensure SDL3_image-shared is correctly linked
    set_target_properties(SDL3_image::SDL3_image-shared PROPERTIES 
        IMPORTED_IMPLIB "C:\\Program Files (x86)\\SDL3_image\\lib\\libSDL3_image.dll.a"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image SDL3::SDL3 gdiplus)

    # Copy resources
    file(COPY ${CMAKE_SOURCE_DIR}/src/win32/cat.ico DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_SOURCE_DIR}/src/cat.png DESTINATION ${CMAKE_BINARY_DIR}/src)

    # Copy DLLs
    file(GLOB DLLS "${CMAKE_SOURCE_DIR}/dlls/*.dll")
    add_custom_target(copy_dlls
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLLS} "${CMAKE_BINARY_DIR}"
        COMMENT "Copying DLLs to build directory"
    )
    add_dependencies(${PROJECT_NAME} copy_dlls)

elseif (LINUX)

    add_executable(${PROJECT_NAME} src/posix/main.c)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

endif()
