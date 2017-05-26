#ifndef _CLEGAL_
#define _CLEGAL_

#include "Util.h"
#include "Row.h"
#include "Placement.h"
#include "GnuplotLivePlotter.h"
#include "GnuplotMatrixPlotter.h"
#include <math.h>
#include <stdio.h>

using namespace std;

class CLegal {
 public:
  CLegal(Placement& placement);
  ~CLegal() {}

  Placement& _placement;
  //GnuplotLivePlotter gnuplotLivePlotter;
  //Solve the legalization, "true" indicates success
  bool solve();
#ifdef ABACUS
  void abacus();
#else
  void exact_forward();
  void exact_backward();
#endif
  void mysolve();
  //Check the legalization result is overlap-free, within boundary, and on-site, "true" indicates legal
  bool check();
  void saveGlobalResult();
  void setLegalResult();
  double totalDisplacement();
  void restoreGlobal();
  //**********************************
  //* Information of Macros or Cells *
  //**********************************
  //vector<int> m_cell_order; //Used as legalization order
  //vector<Row> _free_sites; //Record the unused sites
  //vector<Row> m_left_free_sites;
  //vector<CPoint> m_origLocations;	//Record the original locations
  vector<CPoint> _bestLocs;	//Record the best locations
  vector<CPoint> _bestLocs_forward;	//Record the best locations
  vector<CPoint> _bestLocs_backward;	//Record the best locations
  //vector<Row> m_best_sites;    //Record the resulting sites of the best result
  //vector<CPoint> bestLocs_left;
  //vector<Row> m_best_sites_left;
  vector<CPoint> _globLocs;
  //vector<int> m_macro_ids;
  //vector<double> m_orig_widths;

  vector<int> _modules;

  //Variables and functions used for 4-directional macro shifter
  //Record the original macro positions (left-bottom)
  //vector<CPoint> m_macro_shifter_orig_positions
  //before macro shifter
  //Record the best macro positions (left-bottom)
  //vector<CPoint> m_macro_shifter_best_positions;
  //in 4-directional macro shifter
  //CalculateCellOrder() and CalculateNewLocation() only act on 
  //modules in the m_process_list
  //vector<int> m_process_list;
  //********************************
  //*           parameter          *
  //********************************
  //double m_max_module_height;
  //double m_max_module_width;
  //double m_average_cell_width;    //Average standard cell width
  //Keep the bottom y coordinate of the sites and the
  //height of each site
  //double site_bottom;
  //double site_height;
  //int m_unlegal_count;            //Number of unlegalized cells
  //double _bott_bound, _left_bound;	//Record the chip left boundary
};

#endif 

