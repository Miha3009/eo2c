add_library(model "")

set_target_properties(model PROPERTIES LINKER_LANGUAGE CXX)

target_sources(model
  PRIVATE
    object.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/object.h
    ${CMAKE_CURRENT_LIST_DIR}/types.h
)

target_include_directories(model
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
