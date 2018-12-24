# B\*-Tree with Fast-SA for Solving Fixed-Outline Floorplan Problem

## Requirements
- g++
- gnuplot (optional)
- Boost >= 1.5 (optional)

## Specification
`spec/prog2_floorplanning.pdf`

## File Descriptions
- src/{main.cpp, floor_plan.hpp}: Source files.
- CMakeLists.txt: CMake file for Makefile auto-generation.
- gnuplot-iostream: An iostream version of gnuplot from "http://stahlke.org/dan/gnuplot-iostream" for visualization.

## Compilation
1. First `cd` to the root of this project, then type `cmake .`.
2. CMake will generate a Makefile, then type `make`.
3. A binary program `main` will be generated with compilation flag: g++ -std=c++14 -O3

---

To use iostream-gnuplot: Boost(c++) is required.

## Usage
To generate a result, type
```
./main <Alpha> <Input_block> <Input_nets> <Output_rpt_file> [--plot]
```
- Descriptions of arguments can be found in `spec/prog2_floorplanning.pdf`.
- With `--plot`, the program will visualize B\*-Tree at every step of simulated annealing.
