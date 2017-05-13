#ifndef GNUPLOTMATRIXPLOTTER_H
#define GNUPLOTMATRIXPLOTTER_H

#include <string>
#include <vector>
using namespace std;

class GnuplotMatrixPlotter
{
public:
    GnuplotMatrixPlotter();

    void setMatrix(vector<double> &matrix, unsigned numRows, unsigned numCols);
    void outputPlotFile(string filePathName);
    void outputPngFile(string filePathName);

private:

    vector<double> _matrix;
    unsigned _numRows;
    unsigned _numCols;

    string _title;
};

#endif // GNUPLOTMATRIXPLOTTER_H
