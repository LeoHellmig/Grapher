cmake_minimum_required(VERSION 3.29)

project(grapher)

set(CMAKE_CXX_STANDARD 20)

# A .cpp file is required for the project to be built in CMake
set(SOURCES defines.hpp grapher.cpp grapher.hpp)

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic)
endif()


