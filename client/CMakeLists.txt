
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

add_library(client STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dummy.cpp")

target_include_directories(client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries     (client PUBLIC
    common
    sfml-graphics
    sfml-audio)

# executable

add_executable       (nabu_client "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(nabu_client PRIVATE client)

# tests

if (BUILD_TESTS)
    add_executable(client_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")

    target_link_libraries(client_tests PRIVATE client catch)

    add_test(NAME client_tests COMMAND client_tests)
endif()
