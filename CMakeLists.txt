cmake_minimum_required(VERSION 3.13)

PROJECT("magicparams")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(example)
add_executable(magicparamstests)

target_link_libraries(magicparamstests PRIVATE CONAN_PKG::gtest)

target_include_directories(example PUBLIC include)
target_include_directories(magicparamstests PRIVATE include)

target_sources(example PUBLIC include/magic_params.hpp)
target_sources(example PRIVATE src/example.cpp)
target_sources(magicparamstests PRIVATE tests/test.cpp)