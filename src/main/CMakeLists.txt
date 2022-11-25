cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

add_executable(main main.cpp)

target_link_libraries(main parser)
