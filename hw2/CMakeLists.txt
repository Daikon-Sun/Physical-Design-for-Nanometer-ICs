cmake_minimum_required(VERSION 3.0)

project(FOFP)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pg")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

find_package(Boost COMPONENTS iostreams system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

aux_source_directory(./src SRC)
add_executable(main ${SRC})
target_link_libraries(main ${OpenCV_LIBS} ${Boost_LIBRARIES})
