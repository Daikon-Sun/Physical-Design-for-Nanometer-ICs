![img](https://github.com/Daikon-Sun/Physical-Design-for-Nanometer-ICs-Spring-2017/raw/master/HW4/report/example.png)

*Generated result for case "case3"*

# HW4: Steiner-Tree Construction
- [Efficient Steiner Tree Construction Based on Spanning Graphs](http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=1291582)

## Requirements
- g++ (better >= 5.2 and with OpenMP for multithread)
- gnuplot (optional, for visualization)

## Specification
`spec/prog4_routing.pdf`

## File Descriptions
- `src/\*`: All C/C++ source files.
- `CMakeLists.txt`: a cmake MakeLists for generating Makefile

## Compilation
1. First `cd` to the root of this project, then type `cmake .`.
2. CMake will generate a Makefile, then type `make`.
3. A binary program `steiner_tree` will be generated with compilation flag: `g++ -std=c++11 -O3 -fopenmp`.

## Usage
To generate a result, type
```
./steiner_tree <input_file> <output_file> [output_graph_filename]
```
- `output_graph_filename` is optional, but if provided, then a gnuplot file named `output_graph_filename{i}` will be generated in order to visualize the final result of steiner-tree where the `{i}` implies the plot after `{i}`-th iterations.
