add_executable(eo2c main.cpp)

add_subdirectory(parser)
add_subdirectory(model)
add_subdirectory(generator)

target_link_libraries(eo2c
  PRIVATE
    parser
    model
    generator
)
