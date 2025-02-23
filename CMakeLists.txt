cmake_minimum_required(VERSION 3.30)
project(kittypulse)

if (WIN32) 

    add_executable(${PROJECT_NAME} src/win32/main.c)

endif()
