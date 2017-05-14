#include <cmath>
#include <limits>

#include "Row.h"
#include "Placement.h"

using namespace std;

#ifdef ABACUS
double Row::placeRow(Module& mod, int mod_id, Placement& _pl) {
  vector<Cluster> orig_clusters = _clusters;
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) {
    _clusters.emplace_back(mod.x());
    _clusters.back().addMod(mod, mod_id);
  } else {
    _clusters.back().addMod(mod, mod_id);
    Collapse();
  }
  const auto& clus = _clusters.back();
  double x = clus._xc;
  for(size_t i = 0; i+1<clus._modules.size(); ++i) {
    auto& mod = _pl.module(clus._modules[i]);
    x += mod.width();
  }
  double&& diff = abs(x - mod.x()) + 0.6*abs(_y - mod.y());
  _clusters = orig_clusters;
  return diff;
}
void Row::placeRow_final(Module& mod, int mod_id) {
  _space -= mod.width();
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) {
    _clusters.emplace_back(mod.x());
    _clusters.back().addMod(mod, mod_id);
  } else {
    _clusters.back().addMod(mod, mod_id);
    Collapse();
  }
}
void Row::refresh(Placement& _pl) {
  for(auto& clus : _clusters) {
    double x = clus._xc;
    for(auto& mod_id : clus._modules) {
      auto& mod = _pl.module(mod_id);
      mod.setX(x);
      mod.setY(_y);
      x += mod.width();
    }
  }
}
#else
void Row::placeRow_final(Module& mod, int mod_id, const double& xmin) {
  _space -= mod.width();
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) {
    _clusters.emplace_back(mod, mod_id);
  } else {
    _clusters.back().addMod(mod, mod_id, xmin);
    Collapse(xmin);
  }
}
constexpr double r = 0.6;
double Row::placeRow(Module& mod, const double& xmin) {
  double ydiff = abs(_y - mod.y());
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) return r*ydiff;
  auto& clus = _clusters.back();
  //if number of cell in last cluster is even
  if(clus._modules.size()%2 == 0) 
    return r*ydiff + abs(clus._wc + *clus._it - mod.x());
  //if number of cell in last cluster is odd
  double nxc;
  if(_clusters.size() == 1) {
    if(clus._modules.size() == 1) nxc = max(xmin, mod.x()-clus._wc);
    else nxc = max(*prev(clus._it), mod.x()-clus._wc);
  } else {
    auto& clus2 = _clusters[_clusters.size()-2];
    nxc = max(clus2.right(), max(*prev(clus._it), mod.x()-clus._wc));
  }
  return r*ydiff + abs(nxc + clus._wc - mod.x());
}
void Row::refresh(Placement& _pl) {
  double prvx = 0;
  const double& righ = _pl.boundaryRight();
  for(int j = _clusters.size()-1; j>=0; --j) {
    auto& clus = _clusters[j];
    double x = *clus._it;
    if(j == (int)_clusters.size()-1) x = min(x, righ-clus._wc);
    else x = min(x, prvx-clus._wc);
    prvx = x;
    for(auto& mod_id : clus._modules) {
      auto& mod = _pl.module(mod_id);
      mod.setX(x);
      mod.setY(_y);
      x += mod.width();
    }
  }
}
#endif
