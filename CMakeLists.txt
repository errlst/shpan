cmake_minimum_required(VERSION 3.20)

project(shpan)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(-Wall)

include_directories(public)

add_subdirectory(public)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(bench)
