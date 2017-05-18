#ifndef PLACEMENT_H
#define PLACEMENT_H

#include <cassert>
#include <vector>
#include <string>
using namespace std;

class Placement;

#include "Module.h"
#include "Net.h"
#include "Pin.h"
#include "Row.h"
#include "Rectangle.h"

class Placement {
 public:
  Placement() : _numunFixed(0) {};
  void renew_row_width() { for(auto& row : _rows) row.setWidth(); }
  //void row_assert() {
  //  double h = _rows[0].height();
  //  double ss = _rows[0].siteSpacing();
  //  double w = _rows[0].width();
  //  unsigned ns = _rows[0].numSites();
  //  for(auto& row : _rows) {
  //    assert(row.height() == h);
  //    assert(row.siteSpacing() == ss);
  //    assert(row.width() == w);
  //    assert(row.numSites() == ns);
  //    //if(row.numSites() != 1011)
  //  }
  //  cerr << "good" << endl;
  //}
  /////////////////////////////////////////////
  // input/output
  /////////////////////////////////////////////
  void readBookshelfFormat(const string& filePathName, const string& plFileName);
  void outputGnuplotFigure(const string& filePathName);
  void outputPdfFigure(const string& filePathName);
  void outputPngFigure(const string& filePathName);
  // output file function
  void outputBookshelfFormat(const string& filePathName);
  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  const string& name() {return _name;}
  const string& plname() {return _loadplname;}
  double computeHpwl();
  double computeTotalNetLength(int cellid);
  const Rectangle& rectangleChip() {return _rectangleChip;}  //Chip rectangle
  const double& boundaryTop() {return _boundaryTop;}
  const double& boundaryLeft() {return _boundaryLeft;}
  const double& boundaryBottom() {return _boundaryBottom;}
  const double& boundaryRight() {return _boundaryRight;}
  /////////////////////////////////////////////
  // operation
  /////////////////////////////////////////////
  void moveDesignCenter(const double& xOffset, const double& yOffset);
  /////////////////////////////////////////////
  // get design objects/properties
  /////////////////////////////////////////////
  Module& module(unsigned moduleId) {return _modules[moduleId];}
  Net& net(unsigned netId) {return _nets[netId];}
  Pin& pin(unsigned pinId) {return _pins[pinId];}
  Row& row(unsigned rowId) {return _rows[rowId];}

  const double& getRowHeight() {return _rowHeight;}

  unsigned numModules() {return _modules.size();}
  const unsigned& numunFixed() { return _numunFixed; }
  unsigned numNets() {return _nets.size();}
  unsigned numPins() {return _pins.size();}
  unsigned numRows() {return _rows.size();}
  /////////////////////////////////////////////
  // methods for design (hypergraph) construction
  /////////////////////////////////////////////
  void addModule(const Module &module) {_modules.push_back(module);}
  void addPin(const Pin &pin) {
    _pins.push_back(pin),_pins.back().setPinId(_pins.size());
  }
  void addRow(const Row &row) {_rows.push_back(row);}
  void addDummyModule() {
    _modules.push_back(boundaryLeft());
    _modules.push_back(boundaryRight());
  }
  void removeDummyModule() { _modules.pop_back(); _modules.pop_back(); }

  void setNumModules(unsigned size) {_modules.resize(size);}
  void setnumunFixed(unsigned size) { _numunFixed = size; }
  void setNumNets(unsigned size) {_nets.resize(size);}
  void setNumPins(unsigned size) {_pins.resize(size);}
  void setNumRows(unsigned size) {_rows.resize(size);}

  void clearModules() {_modules.clear();}
  void clearNets() {_nets.clear();}
  void clearPins() {_pins.clear();}
  void clearRows() {_rows.clear();}
  // initialize pins for modules and nets (construct hypergraph)
  void connectPinsWithModulesAndNets();
  ////////////////////
  vector<Row> m_sites; // for Legalization and Detailplace
  vector<Module> modules_bak; //for Detailplace
 private:
  /////////////////////////////////////////////
  // properties
  /////////////////////////////////////////////
  string _name;
  string _loadplname;
  /////////////////////////////////////////////
  // design data
  /////////////////////////////////////////////
  vector<Module> _modules;
  vector<Net> _nets;
  vector<Pin> _pins;
  vector<Row> _rows;
  /////////////////////////////////////////////
  // design statistics
  /////////////////////////////////////////////
  void updateDesignStatistics();
  unsigned _numunFixed;
  Rectangle _rectangleChip;
  double _rowHeight;
  double _boundaryTop;
  double _boundaryLeft;
  double _boundaryBottom;
  double _boundaryRight;
};

#endif // PLACEMENT_H
