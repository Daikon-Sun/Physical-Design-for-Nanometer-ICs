cmake_minimum_required(VERSION 2.8)

project(steiner-tree)
SET(CMAKE_CXX_FLAGS "-Ofast -std=c++11 -Wall -Wextra -fopenmp -ffast-math -march=native")

aux_source_directory(./src SRC)
add_executable(main ${SRC})
target_link_libraries(main)
