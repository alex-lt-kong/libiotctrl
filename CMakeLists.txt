cmake_minimum_required(VERSION 3.18.4)

project(LIBIOTCTRL)

add_compile_options(-Wall -Wextra -pedantic -O3)

add_subdirectory(src/iotctrl)
add_subdirectory(tools)
