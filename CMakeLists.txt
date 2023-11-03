cmake_minimum_required(VERSION 3.18.4)

project(libiotctrl)

# Always generate compile_commands.json for clangd, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/compiler-flags.cmake)
include(cmake/sanitizers.cmake)

include_directories(./src)

add_subdirectory(src/iotctrl)
add_subdirectory(tools)
