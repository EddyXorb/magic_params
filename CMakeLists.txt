cmake_minimum_required(VERSION 3.13)

PROJECT("magicparams")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(magicparams)
add_executable(magicparamstests)

target_link_libraries(magicparamstests PRIVATE CONAN_PKG::gtest)

target_include_directories(magicparams PUBLIC include)
target_include_directories(magicparamstests PRIVATE include)

target_sources(magicparams PUBLIC include/magic_params.hpp)
target_sources(magicparams PRIVATE src/main.cpp)
target_sources(magicparamstests PRIVATE tests/test.cpp)