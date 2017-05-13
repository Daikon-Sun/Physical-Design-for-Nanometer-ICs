#include <cmath>
#include <limits>

#include "Row.h"
#include "Placement.h"

using namespace std;

double Row::placeRow(Module& mod, int mod_id, Placement& _pl) {
  double orig_x = mod.x(), orig_y = mod.y();
  mod.setY(_y);
  vector<Cluster> orig_clusters = _clusters;
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) {
    _clusters.emplace_back(mod.x());
    _clusters.back().addMod(mod, mod_id);
  } else {
    _clusters.back().addMod(mod, mod_id);
    Collapse();
  }
  for(auto& clus : _clusters) {
    double x = clus._xc;
    for(auto& mods : clus._modules) {
      auto& mod = _pl.module(mods);
      mod.setX(x);
      x += mod.width();
    }
  }
  double diff = abs(mod.x() - orig_x) + 0.6*abs(mod.y() - orig_y);
  mod.setX(orig_x);
  mod.setY(orig_y);
  _clusters = orig_clusters;
  return diff;
}
void Row::placeRow_final(Module& mod, int mod_id, Placement& _pl) {
  _space -= mod.width();
  mod.setY(_y);
  if(_clusters.empty() || _clusters.back().right() <= mod.x()) {
    _clusters.emplace_back(mod.x());
    _clusters.back().addMod(mod, mod_id);
  } else {
    _clusters.back().addMod(mod, mod_id);
    Collapse();
  }
  for(auto& clus : _clusters) {
    double x = clus._xc;
    for(auto& mod_id : clus._modules) {
      auto& mod = _pl.module(mod_id);
      mod.setX(x);
      x += mod.width();
    }
  }
}
