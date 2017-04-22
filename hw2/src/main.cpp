#include <cassert>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

clock_t start_time = clock();

#include "floor_plan.hpp"

int main(int argc, char** argv) {
  assert(argc <= 6);
  srand(time(NULL));

  ifstream f_blcks(argv[2], ifstream::in);
  ifstream f_nets(argv[3], ifstream::in);

  int Nnets, Nblcks, W, H;
  string ign;
  f_nets >> ign >> Nnets;
  f_blcks >> ign >> W >> H;
  f_blcks >> ign >> Nblcks;
  bool do_plot = false;
  auto fp =FLOOR_PLAN<ushort, uint>(f_nets, f_blcks, argv, Nnets, Nblcks, W, H);
  fp.init();
  if(do_plot) fp.plot();
  for(int i = 0; i<1; ++i) {
    fp.swap_two_nodes();
    fp.init();
    if(do_plot) fp.plot();
  }
  fp.cost();
  ofstream outs(argv[4], ifstream::out);
  fp.output(outs);
  outs.close();
}
