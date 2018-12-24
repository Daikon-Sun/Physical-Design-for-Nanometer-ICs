cmake_minimum_required(VERSION 2.8)

project(FOFP)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

find_package(Boost COMPONENTS iostreams system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

aux_source_directory(./src SRC)
add_executable(main ${SRC})
target_link_libraries(main ${Boost_LIBRARIES})
