add_library(compiler "")

target_sources(compiler
  PRIVATE
    compiler.cpp
    config.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/compiler.h
    ${CMAKE_CURRENT_LIST_DIR}/config.h
)

target_include_directories(compiler
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(compiler
  PUBLIC
    parser
    model
    optimizer
    generator
)
