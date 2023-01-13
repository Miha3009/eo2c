add_library(generator "")

target_sources(generator
  PRIVATE
    generator.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/generator.h
)

target_include_directories(generator
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(generator
  PRIVATE
    model
)
