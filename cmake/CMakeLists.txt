cmake_minimum_required(VERSION 3.21)

project(GodotDelta LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE GODOTDELTA_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/core/*.cpp"
)

file(GLOB GODOTDELTA_CLI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/cli/*.cpp"
)

add_library(godotdelta_core ${GODOTDELTA_CORE_SOURCES})

target_include_directories(godotdelta_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../cpp
)

add_executable(gddelta ${GODOTDELTA_CLI_SOURCES})

target_link_libraries(gddelta
    PRIVATE
        godotdelta_core
)
