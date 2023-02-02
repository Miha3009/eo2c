add_library(compiler "")

target_sources(compiler
  PRIVATE
    compiler.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/compiler.h
)

target_include_directories(compiler
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(compiler
  PRIVATE
    parser
    model
    generator
)
