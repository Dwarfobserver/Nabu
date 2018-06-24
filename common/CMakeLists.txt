
add_library(common
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dummy.cpp")

target_include_directories(common PUBLIC "${COMMON_INCLUDE_DIR}/common")

# tests

add_executable            (common_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")
target_link_libraries     (common_tests PRIVATE common)
target_include_directories(common_tests PRIVATE ${TESTS_INCLUDES})

add_test(NAME common_tests COMMAND common_tests)
