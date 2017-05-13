1. ID: B03901056
2. Name: 孫凡耕
3. Programming Language: C/C++
4. Compiler: gcc/g++ (better with version >= 5.2)
5. Pack and Compress: tar zcvf b03901056_hw1.tar.gz b03901056_hw1
6. Included Files and Brief Introduction:
	readme.txt: this file
  src/{main.cpp, floor_plan.hpp}: src files
  main: the compiled binary program
  CMakeLists.txt: a cmake MakeLists for generating Makefile
  gnuplot-iostream/: an iostream version of gnuplot from "http://stahlke.org/dan/gnuplot-iostream" for visualization
	report.docx: report in the format Office Open XML.
	report.pdf: report in the format pdf in case of incompatible typesetting.
7. How to Make:
  (1) first cd to the root of this project, then type "cmake ."
  (2) cmake will generate a Makefile, then type "make"
  (3) a binary program "main" will be generated.
	compiler flag: g++ -std=c++14 -O3
  required packages: Boost(c++) (only for iostream-gnuplot)

8. How to Run:
	./<executable file> <alpha> <input_block> <input_nets> <output_rpt_file> [--plot]
