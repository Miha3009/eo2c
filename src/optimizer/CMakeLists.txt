add_library(optimizer "")

target_sources(optimizer
  PRIVATE
    optimizer.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/optimizer.h
)

target_include_directories(optimizer
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(optimizer
  PRIVATE
    model
)
