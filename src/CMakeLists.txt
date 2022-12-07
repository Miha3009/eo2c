add_executable(eo2c main.cpp)

add_subdirectory(parser)

target_link_libraries(eo2c
  PRIVATE
    parser
)
