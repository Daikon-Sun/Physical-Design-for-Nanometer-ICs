#include "Placement.h"
#include "BookshelfParser.h"
#include "GnuplotPlotter.h"
#include "PlaceCommon/Util.h"

#include <limits>
#include <fstream>
#include <iostream>
#include <cassert>
using namespace std;

void Placement::readBookshelfFormat(const string& filePathName, 
                                    const string& plFileName) {
  // set name
  _name = getFileName(filePathName);
  _loadplname = plFileName;
  // read benchmark
  BookshelfParser bookshelfParser(*this);
  if(!bookshelfParser.readAuxFile(filePathName)) exit(0);
  updateDesignStatistics();
}
void Placement::outputBookshelfFormat(const string& filePathName) {
  ofstream plFile(filePathName.c_str());
  if (!plFile) {
    cout << format("[Error] Cannot open pl file \"%s\"\n",
                   filePathName.c_str()) << endl;
  }

  plFile << "UCLA pl 1.0" << endl;
  plFile << endl;
  for(auto& module : _modules) {
    plFile << module.name() << "\t" << module.x() << "  " << module.y() << " : "
           << module.orientString() << endl;
  }
}
void Placement::outputGnuplotFigure(const string& filePathName) {
  GnuplotPlotter plotter;
  plotter.setTitle("placement figure");
  // add rectangles of moudles
  for(auto& module : _modules) plotter.addRectangle(module.rectangle());
  // add rectangle of placement core reigon
  plotter.addRectangle(_rectangleChip);
  plotter.outputPlotFile(filePathName);
}
void Placement::outputPdfFigure(const string& filePathName) {
  GnuplotPlotter plotter;
  plotter.setTitle("placement figure");
  // add rectangles of moudles
  for(auto& module : _modules) plotter.addRectangle(module.rectangle());
  // add rectangle of placement core reigon
  plotter.addRectangle(_rectangleChip);
  plotter.outputPdfFile(filePathName);
}
void Placement::outputPngFigure(const string& filePathName) {
  GnuplotPlotter plotter;
  plotter.setTitle("placement figure");
  // add rectangles of moudles
  for(auto& module : _modules) plotter.addRectangle(module.rectangle());
  // add rectangle of placement core reigon
  plotter.addRectangle(_rectangleChip);
  plotter.outputPngFile(filePathName);
}
double Placement::computeHpwl() {
  double hpwl = 0;

  for (unsigned netId = 0; netId < numNets(); netId++) {
    Net &net = this->net(netId);

    double minX, minY, maxX, maxY;
    minX = minY = numeric_limits<double>::max();
    maxX = maxY =-numeric_limits<double>::max();
    for (unsigned i = 0; i < net.numPins(); i++) {
      Pin &pin = net.pin(i);
      minX = min(minX, pin.x());
      minY = min(minY, pin.y());
      maxX = max(maxX, pin.x());
      maxY = max(maxY, pin.y());
    }
    hpwl += (maxX - minX) + (maxY - minY);
  }
  return hpwl;
}
double Placement::computeTotalNetLength(int cellid) {
  double hpwl = 0;

  Module& curModule = _modules[cellid];
  for (unsigned pin_count = 0; pin_count < curModule.numPins(); ++pin_count) {
    assert(pin_count< curModule.numPins() );

    Pin &pin = curModule.pin(pin_count);
    Net &net = this->net(pin.netId());
    double minX, minY, maxX, maxY;
    minX = minY = numeric_limits<double>::max();
    maxX = maxY =-numeric_limits<double>::max();

    for (unsigned i = 0; i < net.numPins(); ++i) {
      assert(i< net.numPins() );
      Pin &pin = net.pin(i);
      minX = min(minX, pin.x());
      minY = min(minY, pin.y());
      maxX = max(maxX, pin.x());
      maxY = max(maxY, pin.y());
    }
    hpwl += (maxX - minX) + (maxY - minY);
  }
  return hpwl;
}
void Placement::connectPinsWithModulesAndNets() {
  // clear pins for modules and nets
  for (unsigned moduleId = 0; moduleId < _modules.size(); moduleId++)
    _modules[moduleId].clearPins();
  for (unsigned netId = 0; netId < _nets.size(); netId++)
    net(netId).clearPins();
  for (unsigned pinID = 0; pinID < _pins.size(); pinID++)
    pin(pinID).setPinId(pinID);
  // add pins for modules and nets
  for(auto& pin : _pins) {
    module(pin.moduleId()).addPin(&pin);
    net(pin.netId()).addPin(&pin);
  }
}
void Placement::moveDesignCenter(const double& xOffset, const double& yOffset) {
  // move module positions
  for(auto& module : _modules)
    module.setPosition(module.x()+xOffset, module.y()+yOffset);
  // move row positions
  for(auto& row : _rows) {
    row.setPosition(row.x()+xOffset,row.y()+yOffset);
    row.m_interval[0]=row.m_interval[0]+xOffset;
    row.m_interval[1]=row.m_interval[1]+xOffset;
  }
  updateDesignStatistics();
}
void Placement::updateDesignStatistics() {
  // update design bounadries
  double left, bottom, right, top;
  left = bottom = numeric_limits<double>::max();
  right = top = - numeric_limits<double>::max();
  m_sites.clear();
  for(auto& row : _rows) {
    m_sites.push_back(row);
    left   = min(left,   row.x());
    bottom = min(bottom, row.y());
    right  = max(right,  row.x()+row.width());
    top    = max(top,    row.y()+row.height());
  }
  _rectangleChip.setBounds(left, bottom, right, top);
  _boundaryTop=top;
  _boundaryLeft=left;
  _boundaryBottom=bottom;
  _boundaryRight=right;
  // update row height
  _rowHeight = _rows[0].height();
}
