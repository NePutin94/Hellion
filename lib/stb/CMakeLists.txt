cmake_minimum_required(VERSION 3.19)
project(STB)
file(GLOB STB_HEADER RELATIVE "../${CMAKE_SOURCE_DIR}" "../stb/*.h")

add_library(STB INTERFACE ${STB_HEADER})
target_include_directories(STB INTERFACE ".")
