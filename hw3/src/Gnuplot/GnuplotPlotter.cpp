#include "GnuplotPlotter.h"
#include "Util.h"

#include <fstream>
#include <limits>

GnuplotPlotter::GnuplotPlotter()
{
    _autoFit = true;
}

void GnuplotPlotter::outputPngFile(string filePathName)
{
    ofstream file(filePathName.c_str());

    // put both commands and data in a single file (see gnuplot FAQ)
    file << format("set output \"%s.png\" ", filePathName.c_str()) << endl;
    file << format("set term png") << endl;

    file << format("set title \"%s\" ", _title.c_str()) << endl;
    file << format("set size ratio -1") << endl;
    file << format("set nokey") << endl;
    file << "set tics scale 0" << endl;

    // auto fit
    if (_autoFit == true) {
        double left, bottom, right, top;
        left = bottom = numeric_limits<double>::max();
        right = top = - numeric_limits<double>::max();
        for (unsigned i = 0; i < _rectangles.size(); i++) {
            Rectangle &rectangle = _rectangles[i];
            left = min(left, rectangle.left());
            bottom = min(bottom, rectangle.bottom());
            right = max(right, rectangle.right());
            top = max(top, rectangle.top());
        }
        file << format("set xrange[%f:%f]", left, right) << endl;
        file << format("set yrange[%f:%f]", bottom, top) << endl;
    }

    // plot rectangle
    file << "plot '-' with lines linetype 3" << endl;
    for (unsigned i = 0; i < _rectangles.size(); i++) {
        Rectangle &rectangle = _rectangles[i];
        /*
        file << format("%f %f", rectangle.left(), rectangle.bottom()) << endl;
        file << format("%f %f", rectangle.right(), rectangle.bottom()) << endl;
        file << format("%f %f", rectangle.right(), rectangle.top()) << endl;
        file << format("%f %f", rectangle.left(), rectangle.top()) << endl;
        file << format("%f %f", rectangle.left(), rectangle.bottom()) << endl;
        file << endl;
        */

        file << format("%d %d", (int)rectangle.left(), (int)rectangle.bottom()) << endl;
        file << format("%d %d", (int)rectangle.right(), (int)rectangle.bottom()) << endl;
        file << format("%d %d", (int)rectangle.right(), (int)rectangle.top()) << endl;
        file << format("%d %d", (int)rectangle.left(), (int)rectangle.top()) << endl;
        file << format("%d %d", (int)rectangle.left(), (int)rectangle.bottom()) << endl;
        file << endl;
    }
    file << "EOF" << endl;

    system(format("gnuplot %s", filePathName.c_str()).c_str());
}


void GnuplotPlotter::outputPdfFile(string filePathName)
{
    ofstream file(filePathName.c_str());

    // put both commands and data in a single file (see gnuplot FAQ)
    file << format("set output \"%s.pdf\" ", filePathName.c_str()) << endl;
    file << format("set term pdf") << endl;

    file << format("set title \"%s\" ", _title.c_str()) << endl;
    file << format("set size ratio -1") << endl;
    file << format("set nokey") << endl;
    file << "set tics scale 0" << endl;

    // auto fit
    if (_autoFit == true) {
        double left, bottom, right, top;
        left = bottom = numeric_limits<double>::max();
        right = top = - numeric_limits<double>::max();
        for (unsigned i = 0; i < _rectangles.size(); i++) {
            Rectangle &rectangle = _rectangles[i];
            left = min(left, rectangle.left());
            bottom = min(bottom, rectangle.bottom());
            right = max(right, rectangle.right());
            top = max(top, rectangle.top());
        }
        file << format("set xrange[%f:%f]", left, right) << endl;
        file << format("set yrange[%f:%f]", bottom, top) << endl;
    }

    // plot rectangle
    file << "plot '-' with lines linetype 2" << endl;
    for (unsigned i = 0; i < _rectangles.size(); i++) {
        Rectangle &rectangle = _rectangles[i];
        file << format("%f %f", rectangle.left(), rectangle.bottom()) << endl;
        file << format("%f %f", rectangle.right(), rectangle.bottom()) << endl;
        file << format("%f %f", rectangle.right(), rectangle.top()) << endl;
        file << format("%f %f", rectangle.left(), rectangle.top()) << endl;
        file << format("%f %f", rectangle.left(), rectangle.bottom()) << endl;
        file << endl;
    }
    file << "EOF" << endl;

    system(format("gnuplot %s", filePathName.c_str()).c_str());
}

void GnuplotPlotter::outputPlotFile(string filePathName)
{
    ofstream file(filePathName.c_str());

    // put both commands and data in a single file (see gnuplot FAQ)
    file << format("set title \"%s\" ", _title.c_str()) << endl;
    file << format("set size ratio -1") << endl;
    file << format("set nokey") << endl;
    file << "set tics scale 0" << endl;

    // auto fit
    if (_autoFit == true) {
        double left, bottom, right, top;
        left = bottom = numeric_limits<double>::max();
        right = top = - numeric_limits<double>::max();
        for (unsigned i = 0; i < _rectangles.size(); i++) {
            Rectangle &rectangle = _rectangles[i];
            left = min(left, rectangle.left());
            bottom = min(bottom, rectangle.bottom());
            right = max(right, rectangle.right());
            top = max(top, rectangle.top());
        }
        file << format("set xrange[%f:%f]", left, right) << endl;
        file << format("set yrange[%f:%f]", bottom, top) << endl;
    }

    // plot rectangle
    file << "plot '-' with lines" << endl;
    for (unsigned i = 0; i < _rectangles.size(); i++) {
        Rectangle &rectangle = _rectangles[i];
        file << format("%f %f", rectangle.left(), rectangle.bottom()) << endl;
        file << format("%f %f", rectangle.right(), rectangle.bottom()) << endl;
        file << format("%f %f", rectangle.right(), rectangle.top()) << endl;
        file << format("%f %f", rectangle.left(), rectangle.top()) << endl;
        file << format("%f %f", rectangle.left(), rectangle.bottom()) << endl;
        file << endl;
    }
    file << "EOF" << endl;

    // wait
    file << "pause -1 'Press any key'" << endl;
}

void GnuplotPlotter::addRectangle(const Rectangle &rectangle)
{
    _rectangles.push_back(rectangle);
}

void GnuplotPlotter::clearObjects()
{
    _rectangles.clear();
}

void GnuplotPlotter::setTitle(string title)
{
    _title = title;
}
