add_executable(test_all parser_test.cpp validator_test.cpp model_test.cpp helper.cpp)

target_link_libraries(test_all model parser optimizer)

target_include_directories(test_all
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

add_test(
  NAME
    test_all
  COMMAND
    $<TARGET_FILE:test_all>
)