1. ID: B03901056
2. Name: ?]?Z??
3. Programming Language: C/C++
4. Compiler: gcc/g++ (better with version >= 5.2)
5. Pack and Compress: tar zcvf b03901056_hw1.tar.gz b03901056_hw1
6. Included Files and Brief Introduction:
	readme.txt: this file
	orig_fm.cpp: original alrogithm of F-M Heuristic.
	nonzero_fm.cpp: nonzero version of F-M Heuristic.
	multi_orig_fm.cpp: run multiple iterations of original algorithm.
	multi_nonzero_fm.cpp: run multiple iterations of nonzero version.
  four binary files: the compiled executable files for the above cpp files.
	report.docx: report in the format Offict Open XML.
	report.pdf: report in the format pdf in case of incompatible typesetting.
	makefile: a gnu makefile which take cares of compiling those cpp files.
7. How to Make:
	for gcc/g++ version >= 5.2:
		type "make" to make all four cpp files.
		compiler flag: g++ -std=c++14 -O3
	for gcc/g++ version < 5.2:
		type "make old" to make all four cpp files:
		compiler flag: g++ -std=c++1y -O3

8. How to Run:
	./<executable file> [input_file] [output_file]
	For best performance within an hour, please run multi_nonzero_fm.
