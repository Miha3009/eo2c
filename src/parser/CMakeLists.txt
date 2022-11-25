cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(SOURCE_LIB
    parser.h
    parser.cpp
)

add_library(parser STATIC ${SOURCE_LIB})
