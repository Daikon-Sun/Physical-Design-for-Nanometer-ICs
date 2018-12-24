![img](https://github.com/Daikon-Sun/Physical-Design-for-Nanometer-ICs-Spring-2017/raw/master/HW3/report/example.png)

*Generated result for case "ibm01"*

# Abacus-Based Legalizer for Solving Single-Cell Height Legalization
- [Abacus: Fast legalization of standard cell circuits with minimal movement](http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=5642689)
-  [Fast Legalization for Standard Cell Placement with Simultaneous Wirelength and Displacement Minimization](http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=5642689)

## Requirements
- g++ (better >= 5.2)

## Specification
`spec/prog3_legalization.pdf`

## File Descriptions
- `src/\*`: All c/c++ source files.
- `legalizer`: The compiled binary program.
- `Makefile`: A Makefile to generate an executable binary

## Compilation
1. Type `make` in the current directory.
2. A binary program `legalizer` will be generated with compilation flag: `g++ -std=c++11 -O3 -fopenmp -ffast-math -DUSE_OPENMP`.

## Usage
To generate a result, type
```
./legalizer -aux <input>.aux [--plot] [--check]
```
- `--plot` will output a gnuplot `<input>.plt` for visualization.
- `--check` will check whether the solution is legal or not.
- After running, an output file named `<input>.pl` will be saved in the current directory.
