#include <string>
#include <iostream>
#include <cstdio>
using namespace std;

#include "ParamPlacement.h"

CParamPlacement param;	// global variable (2006-03-22) donnie

CParamPlacement::CParamPlacement()
{
    bRunGlobal = true;
    bRunLegal  = true;
    bRunDetail = true;
    plFilename = "";
    threadNum  = 1;
}
