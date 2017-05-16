#include "Placement.h"

#ifndef ROW_H
#define ROW_H

#include <vector>
#include <set>
#include "Module.h"

using namespace std;

#ifdef ABACUS
struct Cluster {
  Cluster(const double& xc) : _xc(xc), _ec(0), _wc(0), _qc(0) {
    _modules.reserve(16);
  }
  double right() { return _xc + _wc; }
  void addMod(Module& mod, int mod_id) { 
    _modules.push_back(mod_id); 
    const double& ei = mod.weight();
    _ec += ei;
    _qc += ei * (mod.x() - _wc);
    _wc += mod.width();
  }
  double _xc, _ec, _wc, _qc;
  vector<int> _modules;
};
#else
struct Cluster2 {
  Cluster2(Module& mod, int mod_id) : _wc(mod.width()) {
    _modules.push_back(mod_id);
    _xcs.insert(mod.x());
    _it = _xcs.begin();
  }
  double right() { return *_it + _wc; }
  void addMod(Module& mod, int mod_id, const double& xmin) {
    double val = max(mod.x() - _wc, xmin);
    _xcs.insert(val);
    if(*_it == val) ++_it;
    if(_xcs.size()%2 == 0) --_it;
    _wc += mod.width();
    _modules.push_back(mod_id);
  }
  double _wc;
  multiset<double>::iterator _it;
  multiset<double> _xcs;
  vector<int> _modules;
};
#endif
class Row {
 public:
  enum Orient {OR_N, OR_W, OR_S, OR_E, OR_FN, OR_FW, OR_FS, OR_FE};
  
  Row(double x = -1, double y = -1, double height = -1, 
      double siteSpacing = -1, unsigned numSites = -1, 
      Orient orient = OR_N, bool isSymmetric = true)
    : _x(x), _y(y), _height(height), _siteSpacing(siteSpacing),
      _numSites(numSites), _orient(orient), _isSymmetric(isSymmetric) {
    _clusters.reserve(20);
  }
#ifdef ABACUS
  void Collapse() {
    auto& clus = _clusters.back();
    assert(clus._ec);
    clus._xc = clus._qc / clus._ec;
    if(clus._xc < _x) clus._xc = _x;
    else if(clus._xc > _x + _width - clus._wc)
      clus._xc = _x + _width - clus._wc;
    if(_clusters.size() > 1
       && _clusters[_clusters.size()-2].right() > clus._xc) {
      Addcluster();
      _clusters.pop_back();
      Collapse();
    }
  }
  void Addcluster() {
    assert(_clusters.size() > 1);
    auto& rev2nd = _clusters[_clusters.size()-2];
    const double& ec = _clusters.back()._ec;
    rev2nd._ec += ec;
    rev2nd._qc += _clusters.back()._qc - ec * rev2nd._wc;
    rev2nd._wc += _clusters.back()._wc;
    rev2nd._modules.insert(rev2nd._modules.end(),
                           _clusters.back()._modules.begin(),
                           _clusters.back()._modules.end());
  }
  double placeRow(Module&, int, Placement&);
  void placeRow_final(Module&, int);
  void refresh(Placement&);
#else
  void Collapse(const double& xmin) {
    auto& clus = _clusters.back();
    double xc = *clus._it;
    if(_clusters.size() > 1 && _clusters[_clusters.size()-2].right() > xc) {
      auto& clus2 = _clusters[_clusters.size()-2];
      bool pre = true;
      for(auto& d : clus._xcs) {
        double val = max(d - clus2._wc, xmin);
        clus2._xcs.insert(val);
        if(val == *clus2._it && pre) ++clus2._it;
        if(&d == &(*clus._it)) pre = false;
      }
      clus2._wc += clus._wc;
      clus2._modules.insert(clus2._modules.end(),
                            clus._modules.begin(), clus._modules.end());
      _clusters.pop_back();
      //Collapse(xmin);
    }
  }
  double placeRow(Module&, const double&);
  void placeRow_final(Module&, int, const double&);
  void refresh(Placement&);
#endif
  const double& space() { return _space; }
  bool enough_space(const double& w) { return _space >= w; }

  vector<double> m_interval;
  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  const double& x() const {return _x;}
  const double& y() const {return _y;}
  const double& height() const {return _height;}
  double width() const {return _numSites*_siteSpacing;}
  const double& siteSpacing() const {return _siteSpacing;}
  const unsigned& numSites() const {return _numSites;}
  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setPosition(double x, double y) { _x = x; _y = y; }
  void setHeight(double height) { _height = height; }
  void setSiteSpacing(double siteSpacing) { _siteSpacing = siteSpacing; }
  void setNumSites(unsigned numSites) { _numSites = numSites; }
  void setOrient(Orient orient) { _orient = orient; }
  void setIsSymmetric(bool isSymmetric) { _isSymmetric = isSymmetric; }
  void setWidth() { _space = _width = width(); }

 private:
  // variables from benchmark input
  double _x, _y; // low x and low y
  double _height; // hieght of row
  double _width; //width of row
  // distance between the beginings of neighboring sites
  double _siteSpacing;
  double _space;
  unsigned _numSites; // number of sites
  Orient _orient; // orient
  bool _isSymmetric; // symmetry
#ifdef ABACUS
  vector<Cluster> _clusters;
#else
  vector<Cluster2> _clusters;
#endif
};

#endif // ROW_H
