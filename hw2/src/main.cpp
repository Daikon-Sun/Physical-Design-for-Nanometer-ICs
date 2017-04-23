#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

clock_t start_time = clock();

#include "floor_plan.hpp"

template<typename ID, typename LEN>
class SA {
public: 
  SA(FLOOR_PLAN<ID, LEN>& fp, ID Nblcks, float R, 
     float P, float alpha_base,float beta) 
    : _fp(fp), _Nblcks(Nblcks), _R(R), _best_sol(fp.get_tree()), _P(P), 
      _alpha_base(alpha_base), _alpha(alpha_base), _beta(beta) {
    _fp.init();
    int hpwl1, max_x1, max_y1; tie(hpwl1, max_x1, max_y1) = _fp.cost();
    float avg_cost = 0;
    _avg_r = float(max_y1)/max_x1, _avg_hpwl = hpwl1, _avg_area = max_x1*max_y1;
    ID N = Nblcks;
    vector<int> hpwls(N+1), areas(N+1);
    vector<float> rs(N+1);
    hpwls[0] = hpwl1, areas[0] = max_x1*max_y1, rs[0] = float(max_y1)/max_x1;
    for(ID i = 1; i<=N; ++i) {
      _fp.perturb();
      _fp.init();
      int hpwl2, max_x2, max_y2; tie(hpwl2, max_x2, max_y2) = _fp.cost();
      _avg_hpwl += hpwl2;
      _avg_area += max_x2*max_y2;
      float r = float(max_y2)/max_x2;
      _avg_r += (r-R)*(r-R);
      hpwls[i] = hpwl2;
      areas[i] = max_x2*max_y2;
      rs[i] = r; 
      tie(hpwl1, max_x1, max_y1) = {hpwl2, max_x2, max_y2};
    }
    _fp.restore(_best_sol);
    _avg_hpwl /= (N+1);
    _avg_area /= (N+1);
    _avg_r /= (N+1);
    _best_cost = sum_norm_cost(hpwls[0], areas[0], rs[0]);
    int cnt = 0;
    for(ID i = 1; i<=N; ++i) {
      float delta_cost = 
          sum_norm_cost(hpwls[i], areas[i], rs[i]) 
        - sum_norm_cost(hpwls[i-1], areas[i-1], rs[i-1]);
      if(delta_cost > 0) avg_cost += delta_cost, ++cnt;
    }
    _T = (!cnt ? 5 : -avg_cost/cnt/logf(P));
  };
  void run() {
    _fp.init();
    bool good = false;
    uint iter = 1;
    _fp.plot();
    cerr << "init cost " << _best_cost << endl;
    cerr << "init T " << _T << endl;
    while(!good && _T > 0.01) {
      ++iter;
      for(uint i = 1; i<=2*_Nblcks; ++i) {
        typename FLOOR_PLAN<ID, LEN>::TREE _cur_sol = _fp.get_tree();
        _fp.perturb();
        _fp.init();
        uint hpwl, max_x, max_y; tie(hpwl, max_x, max_y) = _fp.cost();
        float cost = sum_norm_cost(hpwl, max_x*max_y, float(max_y)/max_x);
        if(cost < _best_cost) {
          _best_sol = _fp.get_tree();
          _best_cost = cost;
        } else if(_rand() > expf((_best_cost-cost)/_T)) _fp.restore(_cur_sol);
      }
      _T /= 1.01;
    }
    cerr << "best cost " << _best_cost << endl;
    cerr << "iter " << iter << endl;
    _fp.restore(_best_sol);
    _fp.init();
    uint hpwl, max_x, max_y; tie(hpwl, max_x, max_y) = _fp.cost();
    float cost = sum_norm_cost(hpwl, max_x*max_y, float(max_y)/max_x);
    cerr << "cost " << cost << endl;
    assert(cost == _best_cost);
    _fp.plot();
  }
private:
  float sum_norm_cost(uint hpwl, uint area, float r) {
    return _alpha*hpwl/_avg_hpwl + _beta*area/_avg_area 
         + (1-_alpha-_beta)*(r-_R)*(r-_R)/_avg_r;
  }
  float _rand() { return float(rand())/RAND_MAX; }
  FLOOR_PLAN<ID, LEN>& _fp;
  typename FLOOR_PLAN<ID, LEN>::TREE _best_sol;
  float _best_cost;
  const uint _Nblcks;
  const float _alpha_base, _P, _R;
  float _alpha, _beta, _T, _avg_hpwl, _avg_area, _avg_r;
};

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
  bool do_plot = true;
  auto fp = FLOOR_PLAN<ushort, uint>(fnets, fblcks, argv, Nnets, Nblcks, W, H);
  float P = 0.9, alpha_base = 0.2, beta = 0, R = float(H)/W;
  auto sa = SA<ushort, uint>(fp, Nblcks, R, P, alpha_base, beta); 
  sa.run();
  //fp.init();
  //if(do_plot) fp.plot();
  //for(int i = 0; i<1; ++i) {
  //  fp.rotate();
  //  fp.init();
  //  fp.plot();
  //}
  fp.cost();
  ofstream outs(argv[4], ifstream::out);
  fp.output(outs);
  outs.close();
}
