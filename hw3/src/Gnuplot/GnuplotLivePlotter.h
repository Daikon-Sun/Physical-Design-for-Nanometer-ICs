#ifndef GNUPLOTLIVEPLOTTER_H
#define GNUPLOTLIVEPLOTTER_H

#include <vector>
#include <string>
#include <cstdio>
using namespace std;

#include "Rectangle.h"

class GnuplotLivePlotter
{
public:
    GnuplotLivePlotter();
    ~GnuplotLivePlotter();

    /////////////////////////////////////////////
    // get
    /////////////////////////////////////////////
    string title() {return _title;}

    /////////////////////////////////////////////
    // set
    /////////////////////////////////////////////
    void setTitle(string title);
    void setAutoFit(bool autoFit) {_autoFit = autoFit;}

    /////////////////////////////////////////////
    // output result
    /////////////////////////////////////////////
    //void outputPlotFile(string filePathName);
    //void outputPdfFile(string filePathName);

    void show();

    /////////////////////////////////////////////
    // add/clear objects
    /////////////////////////////////////////////
    void addRectangle(const Rectangle &rectangle);
    void clearObjects();

    // debug
    void debug();

private:
    // pipe
    FILE *_fpipe;

    // rectangle
    vector<Rectangle> _rectangles;

    // properties
    string _title;
    bool _autoFit;

};

#endif // GNUPLOTLIVEPLOTTER_H
