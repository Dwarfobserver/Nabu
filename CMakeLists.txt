
project(nabu)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(MERGE_CLIENT_SERVER "Merge the client and the server in a single executable" OFF)

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
    add_executable(nabu "${PROJECT_SOURCE_DIR}/merge_main.cpp")
    target_link_libraries(nabu PRIVATE common client server)
endif()
