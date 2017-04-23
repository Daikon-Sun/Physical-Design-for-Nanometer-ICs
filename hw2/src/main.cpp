#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

clock_t start_time = clock();

inline float randf() { return float(rand())/RAND_MAX; }
inline float randb() { return rand()%2; }

#include "floor_plan.hpp"

typedef tuple<uint, uint, uint> uint3;

template<typename ID, typename LEN>
class SA {
public: 
  SA(FLOOR_PLAN<ID, LEN>& fp, ID Nblcks, uint W, uint H, float R, 
     float P, float alpha_base,float beta) 
    : _fp(fp), _Nblcks(Nblcks), _N(Nblcks), _R(R), _best_sol(fp.get_tree()), 
      _P(P), _alpha_base(alpha_base), _alpha(alpha_base), _N_feas(0),
      _beta(beta), _W(W), _H(H) {
    _fp.init();
    vector<uint3> costs(_N+1);
    costs[0] = _fp.cost();
    float avg_cost = 0;
    _avg_r = float(get<2>(costs[0]))/get<1>(costs[0]);
    _avg_hpwl = get<0>(costs[0]), _avg_area = get<1>(costs[0])*get<2>(costs[0]);
    for(ID i = 1; i<=_N; ++i) {
      _fp.perturb();
      _fp.init();
      costs[i] = _fp.cost();
      if(get<1>(costs[i]) < _W && get<2>(costs[i]) < _H) {
        ++_N_feas;
        _recs.push_back(true);
      } else _recs.push_back(false);
      _avg_hpwl += get<0>(costs[i]);
      _avg_area += get<1>(costs[i])*get<2>(costs[i]);
      float r = float(get<2>(costs[i]))/get<1>(costs[i]);
      _avg_r += (r-R)*(r-R);
    }
    _fp.restore(_best_sol);
    _avg_hpwl /= (_N+1);
    _avg_area /= (_N+1);
    _avg_r /= (_N+1);
    _best_cost = sum_norm_cost(costs[0]);
    int cnt = 0;
    for(ID i = 1; i<=_N; ++i) {
      float delta_cost = sum_norm_cost(costs[i]) - sum_norm_cost(costs[i-1]); 
      if(delta_cost > 0) avg_cost += delta_cost, ++cnt;
    }
    _init_T = (!cnt ? 5 : -avg_cost/cnt/logf(P));
  };
  void run(uint k, uint rnd, float c) {
    _fp.init();
    uint iter = 1;
    _fp.plot();
    float _T = _init_T;
    while(_T > 0.01 || iter <= 2*k) {
      float avg_delta_cost = 0;
      for(uint i = 1; i<=rnd; ++i) {
        float prv_cost = sum_norm_cost(_fp.cost());
        typename FLOOR_PLAN<ID, LEN>::TREE _cur_sol = _fp.get_tree();
        _fp.perturb();
        _fp.init();
        uint3 costs = _fp.cost();
        if(*_recs.begin()) --_N_feas;
        _recs.pop_front();
        if(get<1>(costs) < _W && get<2>(costs) < _H) {
          cerr << "feasible!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
          ++_N_feas;
          _recs.push_back(true);
        } else _recs.push_back(false);
        _alpha = _alpha_base + (1-_alpha_base)*_N_feas/_N;
        float cost = sum_norm_cost(costs);
        float delta_cost = cost - prv_cost;
        avg_delta_cost += abs(delta_cost);
        if(delta_cost < 0) {
          if(cost < _best_cost) {
            _best_sol = _fp.get_tree();
            _best_cost = cost;
          }
        } else if(randf() > expf(-delta_cost/_T)) {
          _fp.restore(_cur_sol);
          _fp.init();
        }
      }
      ++iter;
      if(iter <= k) _T = _init_T*avg_delta_cost/rnd/iter/c;
      else _T = _init_T*avg_delta_cost/rnd/iter;
    }
    cerr << "iter " << iter << endl;
    _fp.restore(_best_sol);
    _fp.init();
    float cost = sum_norm_cost(_fp.cost());
    _fp.plot();
  }
private:
  float sum_norm_cost(const uint3& cost) const {
    const float r = float(get<2>(cost))/get<1>(cost);
    return _alpha*get<0>(cost)/_avg_hpwl 
         + _beta*get<1>(cost)*get<2>(cost)/_avg_area 
         + (1-_alpha-_beta)*(r-_R)*(r-_R)/_avg_r
         + (get<1>(cost) > _W || get<2>(cost) > _H);
  }
  FLOOR_PLAN<ID, LEN>& _fp;
  typename FLOOR_PLAN<ID, LEN>::TREE _best_sol;
  float _best_cost;
  const uint _Nblcks, _W, _H, _N;
  const float _alpha_base, _P, _R;
  float _alpha, _beta, _init_T, _avg_hpwl, _avg_area, _avg_r;
  ID _N_feas;
  list<bool> _recs;
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
  float P = 0.95, alpha_base = 0.5, beta = 0, R = float(H)/W;
  auto sa = SA<ushort, uint>(fp, Nblcks, W, H, R, P, alpha_base, beta); 
  uint k = Nblcks, rnd = 100*Nblcks;
  float c = 100;
  sa.run(k, rnd, c);
  ofstream outs(argv[4], ifstream::out);
  fp.output(outs);
  outs.close();
}
