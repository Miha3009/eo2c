cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(eo2c LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../build/bin")

add_subdirectory(src/parser)
add_subdirectory(src/main)