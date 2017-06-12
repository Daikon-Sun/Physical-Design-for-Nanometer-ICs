1. ID: B03901056
2. Name: 孫凡耕
3. Programming Language: C/C++
4. Compiler: g++ (better with version >= 5.2)
5. Pack and Compress: tar -cvf b03901056_hw4.tar.gz b03901056_hw4
6. Included Files and Brief Introduction:
  readme.txt: this file
  src/*: all C/C++ source files
  steiner_tree: the compiled binary program
  CMakeLists.txt: a cmake MakeLists for generating Makefile
  report.docx: report in the format Office Open XML.
  report.pdf: report in the format pdf in case of incompatible typesetting.
7. How to Make:
  (1) first cd to the root of this project, then type "cmake ."
  (2) cmake will generate a Makefile, then type "make"
  (3) a binary program "steiner_tree" will be generated.
  compiler flag: g++ -std=c++11 -O3 -fopenmp
8. How to Run:
  ./<executable file> <input_file> <output_file> [output_graph_filename]
  The fourth argument "[output_graph_filename]" is optional. If provided, then a 
  gnuplot file named "output_graph_filename{i}" will be generated in order to 
  visualize the final result of steiner-tree where the {i} implies the plot after 
  {i}-th iterations.
