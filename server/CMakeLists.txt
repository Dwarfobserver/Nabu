
add_library(server STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/database.cpp")

target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries     (server PUBLIC
    common
    sqlite)

# executable

add_executable       (nabu_server "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(nabu_server PRIVATE server)

# tests

if (BUILD_TESTS)
    add_executable(server_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")

    target_link_libraries(server_tests PRIVATE server catch)

    add_test(NAME server_tests COMMAND server_tests)
endif()
