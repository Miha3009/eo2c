add_library(model "")

set_target_properties(model PROPERTIES LINKER_LANGUAGE CXX)

target_sources(model
  PRIVATE
    object.cpp
    id_tag_table.cpp
    imports_map.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/object.h
    ${CMAKE_CURRENT_LIST_DIR}/types.h
    ${CMAKE_CURRENT_LIST_DIR}/translation_unit.h
    ${CMAKE_CURRENT_LIST_DIR}/id_tag_table.h
    ${CMAKE_CURRENT_LIST_DIR}/imports_map.h
)

target_include_directories(model
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
