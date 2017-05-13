#include "GnuplotLivePlotter.h"

#include "Util.h"

#include <fstream>
#include <limits>

GnuplotLivePlotter::GnuplotLivePlotter()
{
    _autoFit = true;
    _fpipe = (FILE *)popen("gnuplot", "w");
}

GnuplotLivePlotter::~GnuplotLivePlotter()
{
    pclose(_fpipe);
}

void GnuplotLivePlotter::addRectangle(const Rectangle &rectangle)
{
    _rectangles.push_back(rectangle);
}

void GnuplotLivePlotter::clearObjects()
{
    _rectangles.clear();
}

void GnuplotLivePlotter::setTitle(string title)
{
    _title = title;
}

void GnuplotLivePlotter::show()
{
    // set properties
    fprintf(_fpipe, "set title \"%s\"\n", _title.c_str());
    fprintf(_fpipe, "set size ratio -1\n");
    fprintf(_fpipe, "set nokey\n");
    fprintf(_fpipe, "set tics scale 0\n");

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
        fprintf(_fpipe, "set xrange[%f:%f]\n", left, right);
        fprintf(_fpipe, "set yrange[%f:%f]\n", bottom, top);
    }

    // plot rectangle
    fprintf(_fpipe, "plot '-' with lines\n");
    for (unsigned i = 0; i < _rectangles.size(); i++) {
        Rectangle &rectangle = _rectangles[i];

        fprintf(_fpipe, "%d %d\n", (int)rectangle.left(), (int)rectangle.bottom());
        fprintf(_fpipe, "%d %d\n", (int)rectangle.right(), (int)rectangle.bottom());
        fprintf(_fpipe, "%d %d\n", (int)rectangle.right(), (int)rectangle.top());
        fprintf(_fpipe, "%d %d\n", (int)rectangle.left(), (int)rectangle.top());
        fprintf(_fpipe, "%d %d\n", (int)rectangle.left(), (int)rectangle.bottom());
        fprintf(_fpipe, "\n");


    }
    fprintf(_fpipe, "EOF\n");
    fflush(_fpipe);
}

void GnuplotLivePlotter::debug()
{
    this->addRectangle(Rectangle(0,0,10,10));
    this->addRectangle(Rectangle(0,0,5,5));
    this->show();
    this->clearObjects();
    this->addRectangle(Rectangle(0,0,2,2));
    this->show();
}
