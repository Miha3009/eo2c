add_executable(parser_test parser_test.cpp)

target_link_libraries(parser_test model parser)

add_test(
  NAME
    test_parser
  COMMAND
    $<TARGET_FILE:parser_test>
  )