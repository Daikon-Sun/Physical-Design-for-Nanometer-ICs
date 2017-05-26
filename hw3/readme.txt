1. ID: B03901056
2. Name: 孫凡耕
3. Programming Language: C/C++
4. Compiler: gcc/g++ (better with version >= 5.2)
5. Pack and Compress: tar zcvf b03901056_hw2.tar.gz b03901056_hw2
6. Included Files and Brief Introduction:
  readme.txt: this file
  src/*: all c/c++ source files
  legalizer: the compiled binary program
  Makefile: a Makefile to generate an executable binary
  report.docx: report in the format Office Open XML.
  report.pdf: report in the format pdf in case of incompatible typesetting.
7. How to Make:
  (1) type "make" in the current directory
  (2) a binary program "legalizer" will be generated.
  compiler flag: g++ -std=c++11 -O3 -fopenmp -ffast-math -DUSE_OPENMP
8. How to Run:
  ./<executable file> -aux <input.aux> [--plot] [--check]
  Flag "--plot" will output a gnuplot "input.plt" for visualization.
  Flag "--check" will check whether the solution is legal or not.
  p.s. After running, an output file named "input.pl" will be saved in the 
       current directory.
