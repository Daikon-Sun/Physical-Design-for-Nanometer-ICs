#include "Legal.h"
#include "arghandler.h"
#include "GnuplotLivePlotter.h"
#include "GnuplotMatrixPlotter.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <list>
#include <numeric>
#include <set>
using namespace std;

bool CLegal::solve() {
  saveGlobalResult();
  const double& bott_bound = _placement.boundaryBottom();
  const double& rowHeight = _placement.getRowHeight();
  const int&& nRows = _placement.numRows();
  double b_tot = 0, r_tot = nRows * _placement.row(0).width();
  sort(_modules.begin(), _modules.end(), [this](int m1, int m2) { 
         return _placement.module(m1).y() < _placement.module(m2).y();
  for(const auto& mod_id : _modules) b_tot += _placement.module(mod_id).width();
  //GnuplotLivePlotter glp;
  //cerr << "  row total width = " << r_tot << endl;
  //cerr << "block total width = " << b_tot << endl;
  //cerr << "            ratio = " << r_tot / b_tot << endl;
  //cerr << "            ratio = " << b_tot / r_tot << endl;
  //exit(0);

  ////sort(_modules.begin(), _modules.end(), [this](int m1, int m2) { 
  //       return _placement.module(m1).x() < _placement.module(m2).x();
  //     });
  //for(const auto& mod_id : _modules) {
  //  auto& mod = _placement.module(mod_id);
  //  double orig_y = mod.y();
  //  double best_cost[2] = {numeric_limits<double>::max(),
  //                         numeric_limits<double>::max()};
  //  int mid_row_id = (orig_y - bott_bound) / rowHeight + 0.5;
  //  int best_row_id[2] = {-1, -1};
  //  for(int dr = 0; dr<2; ++mid_row_id, ++dr) {
  //    bool suc = false;
  //    int cur_row_id = mid_row_id;
  //    while(true) {
  //      auto& row = _placement.row(cur_row_id);
  //      if(abs(row.y() - orig_y) >= best_cost[dr] && suc) break;
  //      if(row.enough_space(mod.width())) {
  //        double cur_cost = row.placeRow(mod, mod_id, _placement);
  //        if(cur_cost < best_cost[dr] ) {
  //          best_cost[dr] = cur_cost;
  //          best_row_id[dr] = cur_row_id;
  //          suc = true;
  //        }
  //      }
  //      cur_row_id += (dr ? 1 : -1);
  //      if(cur_row_id < 0 || cur_row_id >= nRows) break;
  //    }
  //    if(mid_row_id == nRows-1) break;
  //  }
  //  assert(best_row_id[0] != -1 || best_row_id[1] != -1);
  //  if(best_cost[0] < best_cost[1])
  //    _placement.row(best_row_id[0]).placeRow_final(mod, mod_id, _placement);
  //  else
  //    _placement.row(best_row_id[1]).placeRow_final(mod, mod_id, _placement);
  //}
  //for(const auto& mod_id : _modules) {
  //  const auto& mod = _placement.module(mod_id);
  //  _bestLocs[mod_id] = {mod.x(), mod.y()};
  //}
  setLegalResult();
  if(check()) {
    cout<< "total displacement: " << totalDisplacement() << endl;
    return true;
  } else return false;
}
bool CLegal::check() {
  //cout << "start check" << endl;
  int notInSite=0, notInRow=0, overLap=0;
  ///////////////////////////////////////////////////////
  //1.check all standard cell are on row and in the core region
  //////////////////////////////////////////////////////////
  const double& rowHeight = _placement.getRowHeight();
  for(unsigned int i=0; i<_placement.numModules(); ++i) {
    Module& module = _placement.module(i);
    double curX = module.x();
    double curY = module.y();
    double res = (curY - _placement.boundaryBottom()) / rowHeight;
    //cout << curY << " " << res << endl;
    int ires = (int) res;
    if((_placement.boundaryBottom() + _placement.getRowHeight()*ires) != curY) {
      cerr<<"\nWarning: cell:"<<i<<" is not on row!!";
      ++notInRow;
    }
    if((curY < _placement.boundaryBottom()) 
    || (curX < _placement.boundaryLeft())
    || ((curX+module.width()) > _placement.boundaryRight())
    || ((curY+module.height()) > _placement.boundaryTop())) {
      cerr << "\nWarning: cell:"<<i<<" is not in the core!!";
      cerr << curY << " " << curX << " " << module.width() << " "
           << module.height() << " " << module.name() << endl;
      ++notInSite;
    }
  }
  ///////////////////////////////////////////
  //2. row-based overlapping checking
  ///////////////////////////////////////////
  Rectangle chip = _placement.rectangleChip();
  unsigned numRows = _placement.numRows();
  vector< vector<Module*> > rowModules( numRows, vector<Module*>( 0 ) );
  for(unsigned int i=0; i<_placement.numModules(); ++i) {
    Module& module = _placement.module(i);
    double curY = _bestLocs[i].y;

    if( module.area() == 0 ) continue;

    double yLow = curY - chip.bottom();
    double yHigh= curY + module.height() - chip.bottom();
    size_t low = floor( yLow / rowHeight ), high = floor(yHigh / rowHeight);
    if(fabs( yHigh - rowHeight * floor(yHigh / rowHeight) ) < 0.01) --high;
    for( size_t j = low; j <= high; ++j ) rowModules[j].push_back(&module);
  }
  for(size_t i = 0; i < numRows; ++i) {
    vector<Module*> &modules = rowModules[i];
    sort(modules.begin(), modules.end(), [](Module* a, Module* b){
          return a->x() < b->x();
        });
    if(modules.empty()) continue;
    for(size_t j = 0; j < modules.size() - 1; ++j) {
      Module &mod = *modules[ j ];
      size_t nextId = j+1;
      while( mod.x() + mod.width() > modules[ nextId ]->x() ){
        Module &modNext = *modules[ nextId ];
        if( mod.x() + mod.width() > modules[ nextId ]->x() ){
          ++overLap;
          cout << mod.name() << " overlap with " << modNext.name() << endl;
        }
        ++nextId;
        if(nextId == modules.size()) break;
      }
    }
  }
  //cout << endl <<
  cout <<
    "  # row error: "<<notInRow<<
    "\n  # site error: "<<notInSite<<
    "\n  # overlap error: "<<overLap<< endl;
  //cout << "end of check" << endl;
  if( notInRow!=0 || notInSite!=0 || overLap!=0 ) {
    cout <<"Check failed!!" << endl;
    return false;
  } else {
    cout <<"Check success!!" << endl;
    return true;
  }
}
double CLegal::totalDisplacement() {
  double totaldis = 0;
  for(unsigned moduleId = 0 ; moduleId < _placement.numModules() ; moduleId++) {
    Module& curModule = _placement.module(moduleId);
    double x = curModule.x();
    double y = curModule.y();
    totaldis += CPoint::Distance(_globLocs[moduleId] , CPoint( x, y ));
  }
  return totaldis;
}
CLegal::CLegal(Placement& placement) : _placement(placement) {
  //Compute average cell width
  int cell_count = 0;
  double total_width = 0;
  //double max_height = 0.0;
  //m_max_module_height = 0.0;
  //m_max_module_width = 0.0;
  _modules.resize(placement.numModules());
  iota(_modules.begin(), _modules.end(), 0);
  for(unsigned  moduleId = 0 ; moduleId < placement.numModules() ; moduleId++) {
    Module& curModule = placement.module(moduleId);
    //m_max_module_height = max( m_max_module_height, curModule.height() );
    //m_max_module_width = max( m_max_module_width, curModule.width() );
    //Do not include fixed cells and macros
    if(curModule.isFixed() || curModule.height() > placement.getRowHeight())
      continue;
    cell_count++;
    total_width += curModule.width();
  }
  //m_average_cell_width = total_width / cell_count;
  //_free_sites = placement.m_sites;
  //site_bottom = free_sites.front().y();
  //site_height = free_sites.front().height();
  //initalize m_origLocations and bestLocs
  _bestLocs.resize( placement.numModules() );
  _globLocs.resize( placement.numModules() );
}
void CLegal::saveGlobalResult() {
  for (unsigned moduleId = 0; moduleId < _placement.numModules(); moduleId++) {
    Module &curModule = _placement.module(moduleId);
    double x = curModule.x();
    double y = curModule.y();
    _globLocs[moduleId] = CPoint(x, y);
    _bestLocs[moduleId] = CPoint(x, y);
  }
}
void CLegal::setLegalResult() {
  for (unsigned moduleId = 0; moduleId < _placement.numModules(); moduleId++) {
    Module &curModule = _placement.module(moduleId);
    curModule.setPosition(_bestLocs[moduleId].x, _bestLocs[moduleId].y);
  }
}
