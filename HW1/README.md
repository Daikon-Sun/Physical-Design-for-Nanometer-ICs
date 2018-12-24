# Fiduccia-Mattheyses heuristic for Solving 2-Way, Balanced Partitioning.  

## Requirements
- g++ (better >= 5.2)

## Specification
`spec/prog1_partitioning.pdf`

## File Descriptions
- Makefile: A gnu makefile which take cares of compiling those cpp files.
- orig_fm.cpp: Original algorithm of F-M Heuristic.
- nonzero_fm.cpp: Nonzero version of F-M Heuristic.
- multi_orig_fm.cpp: Run multiple iterations of original algorithm.
- multi_nonzero_fm.cpp: Run multiple iterations of nonzero version.

## Compilation
- type "make" to make all four cpp files.
  - flag for gcc/g++ version >= 5.2: g++ -std=c++14 -O3
  - flag for gcc/g++ version < 5.2: g++ -std=c++1y -O3

## Usage
To generate a result, type
```
./<Executable_file> <Input_file> <Output_file>
```
- Executable_file: One of `orig_fm`, `nonzero_fm`, `multi_orig_fm`, and `multi_nonzero_fm`.
- Input_file: One of the files in directory `input_pa1`.
- Output_file: Output path.

---

To check the correctness of a generated result, see directory `checker`.

## Additional Informations
- For best performance within an hour, run `multi_nonzero_fm`.
- Both `multi_orig_fm` and `multi_nonzero_fm` will search for better solutions
  within about 3000 seconds (because the time limitation in original specification). If it is too long, change the value of `total_sec` in the corresponding file.
