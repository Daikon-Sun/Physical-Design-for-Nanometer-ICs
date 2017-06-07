cmake_minimum_required(VERSION 2.8)

project(steiner-tree)
SET(CMAKE_CXX_FLAGS "-std=c++11 -O3 -fopenmp")

aux_source_directory(./src SRC)
add_executable(steiner_tree ${SRC})
target_link_libraries(steiner_tree)
