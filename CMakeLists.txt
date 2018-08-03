
project(nabu)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

# Handle options

set(VERSION_MAJOR "0" CACHE STRING "The project major version")
set(VERSION_MINOR "1" CACHE STRING "The project minor version")

option(BUILD_CLIENT "Indicates to build the client" ON)
option(BUILD_SERVER "Indicates to build the server" ON)
option(BUILD_TESTS  "Indicates to build the tests"  ON)

if (BUILD_TESTS)
    enable_testing()
endif()

# Show build

if (BUILD_CLIENT)
    if (BUILD_SERVER)
        set(NABU_BUILD_STRING "client and server")
    else()
        set(NABU_BUILD_STRING "client")
    endif()
else()
    if (BUILD_SERVER)
        set(NABU_BUILD_STRING "server")
    else()
        set(NABU_BUILD_STRING "common part")
    endif()
endif()

message(STATUS "Building Nabu ${NABU_BUILD_STRING} (${CMAKE_BUILD_TYPE}) v${VERSION_MAJOR}.${VERSION_MINOR}")

# Organize dependencies

set(DEPS_FMT_DIR   "${PROJECT_SOURCE_DIR}/deps/fmt-5.0.0")
set(DEPS_SFML_DIR  "${PROJECT_SOURCE_DIR}/deps/SFML-2.5.0")
set(DEPS_CATCH_DIR "${PROJECT_SOURCE_DIR}/deps/catch-2.2.2")
set(DEPS_FUN2_DIR  "${PROJECT_SOURCE_DIR}/deps/function2")
set(DEPS_ATT_DIR   "${PROJECT_SOURCE_DIR}/deps/att-1.1.0")
set(DEPS_BOOST_DIR "${PROJECT_SOURCE_DIR}/deps/boost-1.66.0")
set(DEPS_SQL_DIR   "${PROJECT_SOURCE_DIR}/deps/sqlite-3.24.0")

add_library(fmt INTERFACE)
target_include_directories(fmt INTERFACE "${DEPS_FMT_DIR}/include")
target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY)

add_library(catch INTERFACE)
target_include_directories(catch INTERFACE "${DEPS_CATCH_DIR}/include")

add_library(function2 INTERFACE)
target_include_directories(function2 INTERFACE "${DEPS_FUN2_DIR}/include")

add_library(aggregates-to-tuples INTERFACE)
target_include_directories(aggregates-to-tuples INTERFACE "${DEPS_ATT_DIR}/include")

add_library(callable-traits INTERFACE)
add_library(boost::callable-traits ALIAS callable-traits)
target_include_directories(callable-traits INTERFACE "${DEPS_BOOST_DIR}/include")

add_library(sqlite STATIC "${DEPS_SQL_DIR}/src/sqlite3.c")
target_include_directories(sqlite PUBLIC "${DEPS_SQL_DIR}/include")

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "${DEPS_SFML_DIR}/lib/cmake/SFML")

# Add targets

add_subdirectory(common)

if (BUILD_CLIENT)
    add_subdirectory(client)
endif()

if (BUILD_SERVER)
    add_subdirectory(server)
endif()
