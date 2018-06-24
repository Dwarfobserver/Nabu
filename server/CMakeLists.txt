
add_library(server "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

if (MERGE_CLIENT_SERVER)
    target_compile_definitions(server PRIVATE MERGE_SERVER)
endif()

target_include_directories(server PUBLIC
    ${COMMON_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(server PUBLIC
    common)

# executable

if (NOT MERGE_CLIENT_SERVER)
    add_executable(nabu_server "${CMAKE_CURRENT_SOURCE_DIR}/src/empty.cpp")
    target_link_libraries(nabu_server PRIVATE server)
endif()

# tests

add_executable            (server_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")
target_link_libraries     (server_tests PRIVATE server)
target_include_directories(server_tests PRIVATE ${TESTS_INCLUDES})

add_test(NAME server_tests COMMAND server_tests)
