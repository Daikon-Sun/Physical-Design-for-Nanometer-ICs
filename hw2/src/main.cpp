#include <cassert>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

clock_t start_time = clock();

#include "floor_plan.hpp"

//template<typename ID, typename LEN>
//class SA {
//public: 
//  SA(const FLOOR_PLAN<ID, LEN>& fp) : _fp(fp) {};
//private:
//  const FLOOR_PLAN<ID, LEN>& _fp;
//};

int main(int argc, char** argv) {
  assert(argc <= 6);
  srand(time(NULL));

  ifstream fblcks(argv[2], ifstream::in);
  ifstream fnets(argv[3], ifstream::in);

  int Nnets, Nblcks, W, H;
  string ign;
  fnets >> ign >> Nnets;
  fblcks >> ign >> W >> H;
  fblcks >> ign >> Nblcks;
  bool do_plot = false;
  auto fp = FLOOR_PLAN<ushort, uint>(fnets, fblcks, argv, Nnets, Nblcks, W, H);
  
  fp.init();
  //if(do_plot) fp.plot();
  //for(int i = 0; i<1; ++i) {
  //  fp.del_and_ins();
  //  fp.init();
  //  if(do_plot) fp.plot();
  //}
  fp.cost();
  ofstream outs(argv[4], ifstream::out);
  fp.output(outs);
  outs.close();
}
