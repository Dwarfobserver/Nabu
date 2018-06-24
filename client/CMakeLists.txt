
add_library(client "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

if (MERGE_CLIENT_SERVER)
    target_compile_definitions(client PRIVATE MERGE_CLIENT)
endif()

target_compile_definitions(client PUBLIC
    ${GRAPHIC_DEFS})

target_include_directories(client PUBLIC
    ${COMMON_INCLUDE_DIR}
    ${GRAPHIC_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(client PUBLIC
    common
    ${GRAPHIC_LIBRARIES})

# tests

add_executable            (client_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")
target_link_libraries     (client_tests PRIVATE client)
target_include_directories(client_tests PRIVATE ${TESTS_INCLUDES})

add_test(NAME client_tests COMMAND client_tests)
