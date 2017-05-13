#include "GnuplotMatrixPlotter.h"
#include "Util.h"

#include <fstream>

GnuplotMatrixPlotter::GnuplotMatrixPlotter()
{
    _numRows = 0;
    _numCols = 0;
}


void GnuplotMatrixPlotter::setMatrix(vector<double> &matrix, unsigned numRows, unsigned numCols)
{
    _matrix = matrix;
    _numRows = numRows;
    _numCols = numCols;
}

void GnuplotMatrixPlotter::outputPngFile(string filePathName)
{
    ofstream file(filePathName.c_str());

    // put both commands and data in a single file (see gnuplot FAQ)
    file << format("set output \"%s.png\" ", filePathName.c_str()) << endl;
    file << format("set term png") << endl;

    // set properties
    file << format("set title \"%s\" ", _title.c_str()) << endl;
    file << "unset key" << endl;
    file << "set size ratio -1" << endl;
    file << "set tics scale 0" << endl;
    file << "set palette rgbformulae 22,13,-31" << endl;

    // set ranges
    file << format("set xrange[%f:%f]", -0.5, _numRows-0.5) << endl;
    file << format("set yrange[%f:%f]", -0.5, _numCols-0.5) << endl;

    // plot matrix
    file << "plot '-' matrix with image" << endl;
    for (unsigned j = 0; j < _numCols; j++) {
        for (unsigned i = 0; i < _numRows; i++) {
            file << _matrix[i*_numCols + j] << " ";
        }
        file << endl;
    }
    file << "EOF" << endl;
    file << "EOF" << endl;

    system(format("gnuplot %s", filePathName.c_str()).c_str());
}

void GnuplotMatrixPlotter::outputPlotFile(string filePathName)
{
    ofstream file(filePathName.c_str());

    // set properties
    file << format("set title \"%s\" ", _title.c_str()) << endl;
    file << "unset key" << endl;
    file << "set size ratio -1" << endl;
    file << "set tics scale 0" << endl;
    file << "set palette rgbformulae 22,13,-31" << endl;

    // set ranges
    file << format("set xrange[%f:%f]", -0.5, _numRows-0.5) << endl;
    file << format("set yrange[%f:%f]", -0.5, _numCols-0.5) << endl;

    // plot matrix
    file << "plot '-' matrix with image" << endl;
    for (unsigned j = 0; j < _numCols; j++) {
        for (unsigned i = 0; i < _numRows; i++) {
            file << _matrix[i*_numCols + j] << " ";
        }
        file << endl;
    }
    file << "EOF" << endl;
    file << "EOF" << endl;

    // wait
    file << "pause -1 'Press any key'" << endl;
}
