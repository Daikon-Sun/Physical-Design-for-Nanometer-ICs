## **Fiduccia-Mattheyses heuristic** for solving 2-way, balanced partitioning.  

## Requirements
- gcc/g++ >= 5.2

## File Descriptions
- orig_fm.cpp: Original alrogithm of F-M Heuristic.
- nonzero_fm.cpp: Nonzero version of F-M Heuristic.
- multi_orig_fm.cpp: Run multiple iterations of original algorithm.
- multi_nonzero_fm.cpp: Run multiple iterations of nonzero version.
- Report.pdf: Report in the format pdf in case of incompatible typesetting.
- Makefile: A gnu makefile which take cares of compiling those cpp files.

## Compilation
- type "make" to make all four cpp files.
  - flag for gcc/g++ version >= 5.2: g++ -std=c++14 -O3
  - flag for gcc/g++ version < 5.2: g++ -std=c++1y -O3

## Usage
  ```
  ./\<executable_file\> [input_file] [output_file]
  ```
  where executable_file is one of `orig_fm`, `nonzero_fm`, `multi_orig_fm`, and `multi_nonzero_fm`.
  For best performance within an hour, run `multi_nonzero_fm`.
