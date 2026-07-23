cmake_minimum_required(VERSION 3.26)
project(Vulcan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

option(GENERATE_EXAMPLE "Generate an example 3D project with a PBR material sphere." OFF)
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)

add_subdirectory(Vulcan)

if (GENERATE_EXAMPLE)
    add_subdirectory(ExampleGame)
endif()