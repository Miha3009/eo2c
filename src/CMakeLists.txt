add_executable(eo2c main.cpp)

add_subdirectory(parser)
add_subdirectory(model)
add_subdirectory(optimizer)
add_subdirectory(generator)
add_subdirectory(compiler)

target_link_libraries(eo2c
  PRIVATE
    compiler
)
