cmake_minimum_required(VERSION 3.29)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(SDL)

add_library(external INTERFACE)

target_compile_options(SDL3-static PRIVATE "-Wno-deprecated-declarations")

target_link_libraries(external INTERFACE SDL3::SDL3-static)
target_include_directories(external
        INTERFACE SDL/include
        INTERFACE glm)


