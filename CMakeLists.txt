
project(nabu)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

# Handle options

option(MERGE_CLIENT_SERVER
    "Merge the client and the server in a single executable"
    OFF)

add_definitions(-D$<MERGE_CLIENT_SERVER:MERGE_CLIENT_SERVER>)

option(CLIENT_LOGGER_VERBOSITY
    "Specify the client logger verbosity between debug, status (by default), warning and error")
SET   (CLIENT_LOGGER_VERBOSITY "status")

option(SERVER_LOGGER_VERBOSITY
    "Specify the server logger verbosity between debug, status (by default), warning and error")
SET   (SERVER_LOGGER_VERBOSITY "status")

message(STATUS "loglevel=${SERVER_LOGGER_VERBOSITY}")

add_definitions(-DCLIENT_LOGGER_VERBOSITY=${CLIENT_LOGGER_VERBOSITY})
add_definitions(-DSERVER_LOGGER_VERBOSITY=${SERVER_LOGGER_VERBOSITY})

# Organize dependencies

set(DEPS_CATCH_DIR "${PROJECT_SOURCE_DIR}/deps/catch-2.2.2")
set(DEPS_GLEW_DIR  "${PROJECT_SOURCE_DIR}/deps/glew-2.1.0")
set(DEPS_GLFW_DIR  "${PROJECT_SOURCE_DIR}/deps/glfw-3.2.1")

set(DEPS_LIBS_DIR $<$<BOOL:WIN32>:libs/win32>)

if (NOT DEPS_LIBS_DIR)
    message(FATAL_ERROR "Librairies not presents for this platform")
endif()

# Setup common variables

set(COMMON_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/common/include")

set(GRAPHIC_INCLUDES 
    "${DEPS_GLEW_DIR}/include"
    "${DEPS_GLFW_DIR}/include")

set(GRAPHIC_LIBRARIES
    "${DEPS_GLFW_DIR}/${DEPS_LIBS_DIR}/glfw3.lib"
    "${DEPS_GLEW_DIR}/${DEPS_LIBS_DIR}/glew32s.lib"
    $<$<BOOL:WIN32>:opengl32>)

set(GRAPHIC_DEFS GLEW_STATIC)

set(TESTS_INCLUDES "${DEPS_CATCH_DIR}/include")

# Add targets

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)

if (MERGE_CLIENT_SERVER)
    add_executable            (nabu "${PROJECT_SOURCE_DIR}/merge_main.cpp")
    target_include_directories(nabu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries     (nabu PRIVATE client server common)
endif()
