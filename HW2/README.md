![img](https://github.com/Daikon-Sun/Physical-Design-for-Nanometer-ICs-Spring-2017/raw/master/HW2/report/example.png)
*Generated result of case "ami33"*

# B\*-Tree with Fast-SA for Solving Fixed-Outline Floorplan Problem
- [B\*-Trees: A New Representation for Non-Slicing Floorplans](http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=855354)
- [Modern Floorplanning Based on Fast Simulated Annealing](http://cc.ee.ntu.edu.tw/~ywchang/Papers/ispd05-floorplanning.pdf)

## Requirements
- g++
- gnuplot (optional)
- Boost >= 1.5 (optional)

## Specification
`spec/prog2_floorplanning.pdf`

## File Descriptions
- `src/{main.cpp, floor_plan.hpp}`: Source files.
- `CMakeLists.txt`: CMake file for Makefile auto-generation.
- `gnuplot-iostream`: An iostream version of gnuplot from "http://stahlke.org/dan/gnuplot-iostream" for visualization.

## Compilation
1. First `cd` to the root of this project, then type `cmake .`.
2. CMake will generate a Makefile, then type `make`.
3. A binary program `main` will be generated with compilation flag: `g++ -std=c++14 -O3`.

---

To use iostream-gnuplot: gnuplot and Boost(c++) is required.

## Usage
To generate a result, type
```
./main <Alpha> <Input_block> <Input_nets> <Output_rpt_file> [--plot]
```
- Descriptions of arguments can be found in `spec/prog2_floorplanning.pdf`.
- With `--plot`, the program will visualize some steps of simulated annealing and final B\*-Tree.

--- 

To check the correctness of a generated result, type
```
python3 checker.py case_id output_file
```
For example, after executing
```
./main 0.5 input_pa2/1.block input_pa2/1.nets output.rpt
```
you can type
```
python3 checker.py 1 output.rpt
```
to check.
