cmake_minimum_required(VERSION 2.8)

project(steiner-tree)
SET(CMAKE_CXX_FLAGS "-O3 -std=c++11 -fopenmp")

aux_source_directory(./src SRC)
add_executable(main ${SRC})
target_link_libraries(main)
