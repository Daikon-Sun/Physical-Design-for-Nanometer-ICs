#include <cmath>
#include <limits>

#include "Row.h"
#include "Placement.h"

using namespace std;

double Row::placeRow(Module& mod, int mod_id, Placement& _pl) {
  //double orig_x = mod.x(), orig_y = mod.y();
  //mod.setY(_y);
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
  //for(auto& mods : clus._modules) {
  for(size_t i = 0; i+1<clus._modules.size(); ++i) {
    auto& mod = _pl.module(clus._modules[i]);
    //mod.setX(x);
    x += mod.width();
  }
  //double&& diff = abs(mod.x() - orig_x) + 0.6*abs(mod.y() - orig_y);
  double&& diff = abs(x - mod.x()) + 0.6*abs(_y - mod.y());
  //mod.setX(orig_x);
  //mod.setY(orig_y);
  _clusters = orig_clusters;
  return diff;
}
void Row::placeRow_final(Module& mod, int mod_id, Placement& _pl) {
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
