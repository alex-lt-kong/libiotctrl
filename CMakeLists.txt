cmake_minimum_required(VERSION 3.18.4)

project(libiotctrl)

# Always generate compile_commands.json for clangd, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(cmake/compiler-flags.cmake)
include(cmake/sanitizers.cmake)

include_directories(./src/iotctrl)
add_subdirectory(src/iotctrl)
add_subdirectory(src/tools)
