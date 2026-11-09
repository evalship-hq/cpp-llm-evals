cmake_minimum_required(VERSION 3.20)
project(support LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Embed prompts/support.md in the library, and configure again when it changes.
file(READ prompts/support.md SUPPORT_PROMPT)
file(CONFIGURE OUTPUT prompt.hpp CONTENT "inline constexpr char kPrompt[] = R\"prompt(@SUPPORT_PROMPT@)prompt\";\n" @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS prompts/support.md)

add_library(support src/support.cpp)
target_include_directories(support PUBLIC src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(support PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)

add_executable(evalworker src/evalworker.cpp)
target_link_libraries(evalworker PRIVATE support)
