#include "BookshelfParser.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Util.h"

BookshelfParser::BookshelfParser(Placement &placement)
  : _placement(placement) {}
bool BookshelfParser::readAuxFile(string filePathName) {
  // open file
  ifstream auxFile(filePathName.c_str());
  if (!auxFile) { 
    cerr << format("[Error] Cannot open aux file \"%s\"", filePathName.c_str())
         << endl;
    return false;
  }
  string fileFormat;
  // read file format
  auxFile >> fileFormat;
  if (fileFormat != "RowBasedPlacement")
    cout << format("[Warning] Format \"%s\" is not supported",
                   fileFormat.c_str()) << endl;
  string nodesFileName;
  string netsFileName;
  string wtsFileName;
  string sclFileName;
  string plFileName;
  // read file names
  string token;
  while (auxFile >> token) {
    size_t dotPos = token.rfind('.');
    if (dotPos != string::npos) {
      string suffix = token.substr(dotPos + 1);
      if (suffix == "nodes") nodesFileName = token;
      else if(suffix == "nets") netsFileName = token;
      else if(suffix == "wts") wtsFileName = token;
      else if(suffix == "scl") sclFileName = token;
      else if(suffix == "pl" ) plFileName = token;
    }
  }
  // get the path of file
  string filePath;
  size_t slashPos = filePathName.rfind('/');
  if (slashPos != string::npos) filePath = filePathName.substr(0, slashPos + 1);
  /////////////////////////////////////////////
  // read benchmark files
  /////////////////////////////////////////////
  readSclFile(filePath + sclFileName);
  readNodesFile(filePath + nodesFileName);
  readNetsFile(filePath + netsFileName);
  _placement.connectPinsWithModulesAndNets();
  if(_placement.plname()!="") {
    plFileName = _placement.plname();
    readPlFile(plFileName);
    cout<<"load pl file: "<<plFileName<<endl;
  } else readPlFile(filePath + plFileName);
  return true;
}
bool BookshelfParser::readNodesFile(const string& filePathName) {
  ifstream nodesFile(filePathName.c_str());
  if (!nodesFile) {
    cerr << format("[Error] Cannot open nodes file \"%s\"\n",
                   filePathName.c_str()) << endl;
    return false;
  }
  // number of nodes
  int numNodes = -1;
  int numTerminals = -1;
  // number of nodes read
  int numNodesRead = 0; // numNodeRead = numModuleRead + numTerminalRead
  int numModulesRead = 0;
  int numTerminalsRead = 0;

  string line, token;
  // read one line at a time (ignore line 1)
  getline(nodesFile, line);
  while (getline(nodesFile, line)) {
    // ignore comments and empty lines
    if(line[0] == '#' || line == "") continue;
    // data for each module
    string nodeName;
    double width = -1;
    double height = -1;
    bool terminal = false;
    // tokenize the line
    stringstream strStream(line);
    strStream >> token;
    if (token == "NumNodes") {
      strStream >> token >> numNodes;
      _placement.setNumModules(numNodes);
    } else if (token == "NumTerminals") {
      strStream >> token >> numTerminals;
    } else {
      // read data
      nodeName = token;
      while (strStream >> token) {
        if (token == "terminal" || token == "terminal_NI") terminal = true;
        else if(token == "symmetry")// ignore symmetry
          cout << "[Warning] symmetry in .nodes is not supported" << endl;
        else {
          width = atof(token.c_str());
          strStream >> height;
        }
      }
      // add a new module
      Module &module = _placement.module(numNodesRead);
      module.setName(nodeName);
      module.setWidth(width);
      module.setHeight(height);
      module.setIsFixed(terminal);
      numNodesRead++;
      if (terminal) numTerminalsRead++;
      else numModulesRead++;
    }
  }
  /////////////////////////////////////////////
  // check number consistency
  /////////////////////////////////////////////
  if(numModulesRead + numTerminalsRead != numNodes) {
    cout << "[Warning] numNodes in .nodes do not "
            "match the number of nodes read" << endl;
    cout << numModulesRead << " " << numTerminalsRead <<" " << numNodes << endl;
  }
  if (numTerminalsRead != numTerminals)
    cout << "[Warning] numTerminals in .nodes do not "
            "match the number of terminals read" << endl;
  /////////////////////////////////////////////
  // build nodeNameTomoduleId map
  /////////////////////////////////////////////
  for (unsigned i = 0; i < _placement.numModules(); i++)
    _nodeNameToModuleId[_placement.module(i).name()] = i;

  /////////////////////////////////////////////
  // debug
  /////////////////////////////////////////////
  //if (false) {
  //  for (unsigned i = 0; i < _placement.numModules(); i++) {
  //    Module &module = _placement.module(i);
  //    cout << module.name() << " "
  //      << module.width() << " " << module.height() << endl;
  //  }
  //}
  return true;
}
bool BookshelfParser::readNetsFile(const string& filePathName) {
  ifstream netsFile(filePathName.c_str());
  if (!netsFile) {
    cerr << format("[Error] Cannot open nets file \"%s\"\n",
                   filePathName.c_str()) << endl;
    return false;
  }
  // numbers of nets and pins
  unsigned numNets = -1;
  unsigned numPins = -1;
  // number of nets and pins read
  unsigned numNetsRead = 0;
  unsigned numPinsRead = 0;
  // data for each net
  int netDegree;
  string netName;
  string nodeName;
  string pinDirection;
  double xOffset;
  double yOffset;

  string line, token;
  // read one line at a time (ignore line 1)
  getline(netsFile, line);
  while (getline(netsFile, line)) {
    // ignore comments and empty lines
    if(line[0] == '#' || line == "") continue;
    // tokenize the line
    stringstream strStream(line);
    strStream >> token;
    if (token == "NumNets") {
      strStream >> token >> numNets;
      _placement.setNumNets(numNets);
    } else if (token == "NumPins") {
      strStream >> token >> numPins;
      _placement.setNumPins(numPins);
    } else if(token == "NetDegree") {
      strStream >> token >> netDegree >> netName;
      // read net
      for (int i = 0; i < netDegree; i++) {
        // default offsets
        xOffset = 0;
        yOffset = 0;
        // read a single pin
        getline(netsFile, line);
        stringstream strStream(line);
        strStream >> nodeName >> pinDirection >> token
                  >> xOffset >> yOffset;
        // add pin
        unsigned moduleId = _nodeNameToModuleId[nodeName];
        Pin &pin = _placement.pin(numPinsRead);
        pin.setModuleId(moduleId);
        pin.setNetId(numNetsRead);
        pin.setOffset(xOffset, yOffset);
        numPinsRead++;
      }
      numNetsRead++;
    }
  }
  /////////////////////////////////////////////
  // check number consistency
  /////////////////////////////////////////////
  if (numNetsRead != numNets)
    cout << "[Warning] numNets in .nets do not "
            "match the number of nets read" << endl;
  if (numPinsRead != numPins)
    cout << "[Warning] numPins in .nets do not "
            "match the number of pins read" << endl;
  /////////////////////////////////////////////
  // debug
  /////////////////////////////////////////////
  //if (false) {
  //  for (unsigned pinId = 0; pinId < _placement.numPins(); pinId++) {
  //    Pin &pin = _placement.pin(pinId);
  //    cout << _placement.module(pin.moduleId()).name() << " "
  //      << pin.xOffset() << " " << pin.yOffset() << endl;
  //  }
  //}
  return true;
}
bool BookshelfParser::readSclFile(const string& filePathName) {
  ifstream sclFile(filePathName.c_str());
  if (!sclFile) {
    cerr << format("[Error] Cannot open scl file \"%s\"\n",
                   filePathName.c_str()) << endl;
    return false;
  }
  // number of rows
  unsigned numRows = -1;
  // number of rows read
  unsigned numRowsRead = 0;
  // data for each row
  double coordinate = -1;
  double height = -1;
  double siteSpacing = -1;
  Row::Orient siteOrient = Row::OR_N;
  bool siteSymmetry = true;
  double subRowOrigin = -1;
  int numSites = -1;
  // variable for counting number of sub-rows in each row
  int numSubRowRead = -1;

  string line, token;
  // read one line at a time (ignore line 1)
  getline(sclFile, line);
  while (getline(sclFile, line)) {
    // ignore comments and empty lines
    if(line[0] == '#' || line == "") continue;
    // tokenize the line
    stringstream strStream(line);
    strStream >> token;
    if (token == "Numrows" || token == "NumRows") {
      strStream >> token >> numRows;
      _placement.setNumRows(numRows);
    } else if (token == "CoreRow") {
      strStream >> token;
      if (token != "Horizontal")
        cout << format("[Warning] CoreRow format \"%s\" in .scl"
                       "is not supported", token.c_str()) << endl;
      // clear data
      coordinate = -1;
      height = -1;
      siteSpacing = -1;
      siteOrient = Row::OR_N;
      siteSymmetry = true;
      subRowOrigin = -1;
      numSites = -1;
      // reset number of sub-rows
      numSubRowRead = 0;
    } else if (token == "Coordinate") {
      strStream >> token >> coordinate;
    } else if (token == "Height") {
      strStream >> token >> height;
    } else if (token == "Sitewidth") {
      // ignore this should be ok
    } else if (token == "Sitespacing") {
      strStream >> token >> siteSpacing;
    } else if (token == "Siteorient") {
      strStream >> token >> token;
      if (token == "N") siteOrient = Row::OR_N;
      else if (token == "W") siteOrient = Row::OR_W;
      else if (token == "S") siteOrient = Row::OR_S;
      else if (token == "E") siteOrient = Row::OR_E;
      else if (token == "FN") siteOrient = Row::OR_FN;
      else if (token == "FW") siteOrient = Row::OR_FW;
      else if (token == "FS") siteOrient = Row::OR_FS;
      else if (token == "FE") siteOrient = Row::OR_FE;
      
    } else if (token == "Sitesymmetry") {
      strStream >> token >> token;
      if (token == "Y") siteSymmetry = true;
      else siteSymmetry = false;
    } else if (token == "SubrowOrigin") {
      // check the assumption that only one sub-row in each row
      if (++numSubRowRead > 1)
        cout << "[Warning] number of sub-rows > 1 in "
                ".scl is not supported" << endl;
      strStream >> token >> subRowOrigin >> token >> token >> numSites;
    } else if (token == "End") {
      // add a new row
      if (numSubRowRead == 1) {
        Row &row = _placement.row(numRowsRead);
        row.setPosition(subRowOrigin, coordinate);
        row.setHeight(height);
        row.setSiteSpacing(siteSpacing);
        row.setNumSites(numSites);
        row.setOrient(siteOrient);
        row.setIsSymmetric(siteSymmetry);
        row.m_interval.push_back(subRowOrigin);
        row.m_interval.push_back(subRowOrigin+numSites*siteSpacing);
        numRowsRead++;
      }
    }
  } // end while
  /////////////////////////////////////////////
  // check number consistency
  /////////////////////////////////////////////
  if (numRowsRead != numRows)
    cout << "[Warning] numRows in .scl do not "
            "match the number of rows read" << endl;
  /////////////////////////////////////////////
  // debug
  /////////////////////////////////////////////
  //if (false) {
  //  for (unsigned i = 0; i < _placement.numRows(); i++) {
  //    Row &row = _placement.row(i);
  //    cout << row.x() << " " << row.y() << " " << row.height() << endl;
  //  }
  //}
  return true;
}
bool BookshelfParser::readPlFile(const string& filePathName) {
  ifstream plFile(filePathName.c_str());
  if (!plFile) {
    cerr << format("[Error] Cannot open pl file \"%s\"\n",
                   filePathName.c_str()) << endl;
    return false;
  }
  // data
  string nodeName;
  double x;
  double y;
  Module::Orient moduleOrient = Module::OR_N;

  string line, token;
  // read one line at a time (ignore line 1)
  getline(plFile, line);
  while (getline(plFile, line)) {
    // ignore comments and empty lines
    if(line[0] == '#' || line == "") continue;
    // read data
    stringstream strStream(line);
    strStream >> nodeName >> x >> y >> token >> token;
    if (token == "N") moduleOrient = Module::OR_N;
    else if (token == "W") moduleOrient = Module::OR_W;
    else if (token == "S") moduleOrient = Module::OR_S;
    else if (token == "E") moduleOrient = Module::OR_E;
    else if (token == "FN") moduleOrient = Module::OR_FN;
    else if (token == "FW") moduleOrient = Module::OR_FW;
    else if (token == "FS") moduleOrient = Module::OR_FS;
    else if (token == "FE") moduleOrient = Module::OR_FE;
    /////////////////////////////////////////
    // set module position
    /////////////////////////////////////////
    unsigned moduleId = _nodeNameToModuleId[nodeName];
    Module &module = _placement.module(moduleId);
    module.setPosition(x, y);
    module.setOrient(moduleOrient);
  }
  return true;
}
