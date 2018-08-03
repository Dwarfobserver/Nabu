
find_package(SFML 2.5 COMPONENTS network REQUIRED)

add_library(common STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/reflection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/reactor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/allocators.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.cpp")

target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries     (common PUBLIC
    fmt
    function2
    aggregates-to-tuples
    sfml-network
    boost::callable-traits)

target_compile_definitions(common PUBLIC
    NABU_VERSION_MAJOR=${VERSION_MAJOR}
    NABU_VERSION_MINOR=${VERSION_MINOR})

# tests

if (BUILD_TESTS)
    add_executable(common_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/serialization.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser.cpp")

    target_link_libraries(common_tests PRIVATE common catch)

    add_test(NAME common_tests COMMAND common_tests)
endif()
