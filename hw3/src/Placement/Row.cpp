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
constexpr double r = 0.7;
constexpr double alpha = 1.1;
constexpr double beta = 0.1;
double Row::placeRow(Module& mod, const double& xmin) {
  double ydiff = r * abs(_y - mod.y());
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) return ydiff;
  auto& clus = _clusters.back();
  //if number of cell in last cluster is even
  if(clus._modules.size()%2 == 0)
    return ydiff + alpha*abs(clus.right() - mod.x());
  //if number of cell in last cluster is odd
  if(_clusters.size() == 1) {
    if(clus._modules.size() == 1) {
      double v1 = max(xmin, mod.x()-clus._wc);
      return ydiff + alpha*abs(v1 + clus._wc - mod.x()) + beta*(*clus._it-v1);
    } else {
      double v1 = *prev(clus._it);
      double&& v2 = mod.x()-clus._wc;
      return ydiff + alpha*abs(max(v1, v2) + clus._wc - mod.x())
                   + beta*(*clus._it-max(v1, v2))*clus._modules.size();
    }
  } else {
    auto& clus2 = _clusters[_clusters.size()-2];
    double v1 = max(*prev(clus._it), mod.x()-clus._wc);
    double&& v2 = clus2.right();
    return ydiff + alpha*abs(max(v1, v2) + clus._wc - mod.x())
                 + beta*(*clus._it - max(v1, v2))*clus._modules.size();
  }
  __builtin_unreachable();
}
void Row::refresh(Placement& _pl) {
  double righ = _pl.boundaryRight();
  for(int j = _clusters.size()-1; j>=0; --j) {
    auto& clus = _clusters[j];
    double x = *clus._it;
    if(j == (int)_clusters.size()-1) x = min(x, righ-clus._wc);
    else x = min(x, _pl.module(_clusters[j+1]._modules[0]).x()-clus._wc);
    for(auto& mod_id : clus._modules) {
      auto& mod = _pl.module(mod_id);
      mod.setX(x);
      mod.setY(_y);
      x += mod.width();
    }
  }
}
#endif
