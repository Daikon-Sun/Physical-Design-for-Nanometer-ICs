#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

#include "Pin.h"
#include "Rectangle.h"

class Module {
 public:
  enum Orient {OR_N, OR_W, OR_S, OR_E, OR_FN, OR_FW, OR_FS, OR_FE};

  Module(string name = "" , double width = -1, double height = -1, 
         bool isFixed = false)
    : _name(name), _x(-1), _y(-1), _width(width), _height(height),
      _isFixed(isFixed), _orient(OR_N) {}
  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  const string& name() {return _name;}
  const double& x() const {return _x;} //x coordinates
  const double& y() const {return _y;} //y coordinates //(x,y): lower-left point of the block
  const double& width() {return _width;}
  const double& height() {return _height;}
  const bool& isFixed() {return _isFixed;} //if fixed module, return true

  double centerX() {return _x + _width/2;}
  double centerY() {return _y + _height/2;}
  double area() {return _width * _height;}
  //module rectangle
  Rectangle rectangle() { return Rectangle(_x, _y, _x+_width, _y+_height);}

  const Orient& orient() {return _orient;}
  string orientString() {
    const char *orientString[] = {"N", "W", "S", "E", "FN", "FW", "FS", "FE"};
    return orientString[_orient];
  }
  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setName(const string &name) { _name = name; }
  void setPosition(const double& x, const double& y) {
    //would update the pin positions when you set new position
    _x = x;
    _y = y;
    updatePinPositions(); //update pin positions
  }
  void setCenterPosition(const double& x, const double& y) {
    _x = x - _width/2;
    _y = y - _height/2;
    updatePinPositions();
  }
  void setX(const double& x) { _x = x; }
  void setY(const double& y) { _y = y; }
  void setWidth(double width) { _width = width; }
  void setHeight(double height) { _height = height; }
  void setIsFixed(bool isFixed) { _isFixed = isFixed; }
  void setOrient(Orient orient) {
    // swap width and height
    if ((_orient % 2) != (orient % 2)) swap(_width, _height);
    // flip back
    if (_orient >= 4)
      for(auto& pin : _pPins) pin->setOffset(-pin->xOffset(), pin->yOffset());
    // rotate 90 degree (counter-clockwise)
    int rotateDistance = (8 + orient - _orient) % 4;
    for(auto& pin : _pPins) {
      for (int k = 0; k < rotateDistance; k++) {
        double xOffset = pin->xOffset();
        double yOffset = pin->yOffset();
        pin->setOffset(-yOffset, xOffset);
      }
    }
    // flip if needed
    if (orient >= 4)
      for(auto& pin : _pPins) pin->setOffset(-pin->xOffset(), pin->yOffset());
    updatePinPositions();
    // change orient
    _orient = orient;
  }
  /////////////////////////////////////////////
  // get (for pins of this modules)
  /////////////////////////////////////////////
  unsigned numPins() {return _pPins.size();}
  Pin& pin(unsigned index) {return *_pPins[index];}
  /////////////////////////////////////////////
  // set (for pins of this modules)
  /////////////////////////////////////////////
  void setNumPins(unsigned numPins) {_pPins.resize(numPins);}
  void addPin(Pin *pPin) {_pPins.push_back(pPin);}
  void clearPins() {_pPins.clear();}
  double weight() { return 1.0/_width; }
 private:
  // variables from benchmark input
  string _name;
  double _x, _y; // low x and low y
  double _width, _height;
  bool _isFixed;
  Orient _orient;
  // pins of the module
  vector<Pin*> _pPins;
  // update pin positions
  void updatePinPositions() {
    for(auto& pin : _pPins)
      pin->setPosition(centerX()+pin->xOffset(), centerY()+pin->yOffset());
  }
};

#endif // MODULE_H
