add_library(generator "")

target_sources(generator
  PRIVATE
    generator.cpp
    application_generator.h
    application_generator.cpp
    code_model.h
    code_model.cpp
    cmake_generator.cpp
    entry_point_generator.cpp
    util.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/generator.h
    ${CMAKE_CURRENT_LIST_DIR}/cmake_generator.h
    ${CMAKE_CURRENT_LIST_DIR}/entry_point_generator.h
)

target_include_directories(generator
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(generator
  PRIVATE
    model
)
