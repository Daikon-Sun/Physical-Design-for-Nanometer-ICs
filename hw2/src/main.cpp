#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>

using namespace std;

clock_t start_time = clock();

inline float randf() { return float(rand())/RAND_MAX; }
inline float randb() { return rand()%2; }

typedef tuple<uint, uint, uint> uint3;

#include "floor_plan.hpp"

#define float double

template<typename T, typename U>
void plot_2d(vector<T>& data, vector<U> axis, bool logscale, uint limit) {
  assert(axis.size() >= limit || axis.size() == data.size());
  Gnuplot gp;
  if(logscale) gp << "set logscale y 2" << endl;
  gp << "set nokey" << endl;
  gp << "set style line 1 lc rgb \'#0060ad\' pt 6" << endl;
  for(uint i = 1; i<data.size() && i<limit; ++i)
    gp << "set arrow " << i << " from " << axis[i-1] << "," << data[i-1]
       << " to " << axis[i] << "," << data[i] << " nohead" << endl;
  gp << "plot '-' w p ls 1" << endl;
  for(uint i = 0; i<data.size() && i<limit; ++i)
    gp << axis[i] << " " << data[i] << endl;
  gp << "e\npause -1" << endl;
}

template<typename ID, typename LEN>
class SA {
public: 
  SA(FLOOR_PLAN<ID, LEN>& fp, ID Nblcks, uint W, uint H, float R, 
     float P, float alpha_base,float beta) 
    : _fp(fp), _Nblcks(Nblcks), _N(Nblcks), _R(R), _best_sol(fp.get_tree()), 
      _alpha_base(alpha_base), _alpha(alpha_base), _N_feas(0),
      _beta(beta), _W(W), _H(H) {
    _fp.init();
    vector<uint3> costs(_N+1);
    costs[0] = _fp.cost();
    _avg_r = float(get<2>(costs[0]))/get<1>(costs[0]);
    _avg_hpwl = get<0>(costs[0]), _avg_area = get<1>(costs[0])*get<2>(costs[0]);
    for(ID i = 1; i<=_N; ++i) {
      _fp.perturb();
      _fp.init();
      costs[i] = _fp.cost();
      if(get<1>(costs[i]) <= _W && get<2>(costs[i]) <= _H) {
        ++_N_feas;
        _recs.push_back(true);
      } else _recs.push_back(false);
      _avg_hpwl += get<0>(costs[i]);
      _avg_area += get<1>(costs[i])*get<2>(costs[i]);
      const float r = float(get<2>(costs[i]))/get<1>(costs[i]);
      _avg_r += (r-R)*(r-R);
    }
    _fp.restore(_best_sol);
    _avg_hpwl = _avg_hpwl*1.001/(_N+1);
    _avg_area = _avg_area*1.001/(_N+1);
    _avg_r = _avg_r*1.001/(_N+1);
    _best_cost = norm_cost(costs.back());
    float avg_cost = 0;
    for(ID i = 1; i<=_N; ++i)
      avg_cost += abs(norm_cost(costs[i]) - norm_cost(costs[i-1]));
    _init_T = -avg_cost/_N/logf(P);
  };
  void run(const uint k, const uint rnd, const float c) {
    clock_t tt = clock();
    vector<float> Ts(1, _init_T), bests(1, _best_cost), ax(1, 0);
    _fp.init();
    uint iter = 1, tot_feas = 0;
    float _T = 2*_init_T, prv_cost = norm_cost(_fp.cost());
    uint rej_num = 0, cnt = 1;
    typename FLOOR_PLAN<ID, LEN>::TREE last_sol = _fp.get_tree();
    while(_T > 0.01 || float(rej_num) <= 0.99f*cnt || !tot_feas) {
      float avg_delta_cost = 0;
      rej_num = 0, cnt = 1;
      for(; cnt<=rnd; ++cnt) {
        _fp.perturb();
        _fp.init();
        uint3 costs = _fp.cost();
        float cost = norm_cost(costs);
        float delta_cost = (cost - prv_cost);

        if(*_recs.begin()) --_N_feas;
        _recs.pop_front();
        if(feas(costs)) {
          ++_N_feas;
          _recs.push_back(true);
          ++tot_feas;
        } else _recs.push_back(false);
        _alpha = _alpha_base + (1-_alpha_base)*_N_feas/_N;
        avg_delta_cost += abs(delta_cost);
        if(delta_cost <= 0 || randf() < expf(-delta_cost/_T)) {
          prv_cost = cost;
          last_sol = _fp.get_tree();
          if(feas(costs)) {
            if(cost < _best_cost) {
              _best_sol = _fp.get_tree();
              _best_cost = cost;
              bests.push_back(_best_cost);
              ax.push_back(float(clock()-tt)/CLOCKS_PER_SEC);
            }
          }
        } else {
          _fp.restore(last_sol);
          _fp.init();
          ++rej_num;
        }
      }
      ++iter;
      if(iter <= k) _T = _init_T*avg_delta_cost/cnt/iter/c;
      else _T = _init_T*avg_delta_cost/cnt/iter;
      Ts.push_back(_T);
      _fp.init();
      if(!tot_feas) {
        if(iter > 30*_Nblcks) {
          _T = _init_T;
          iter = 1;
          cerr << "reset..." << endl;
        }
      } else if(iter > 80*_Nblcks) break;
    }
    cerr << "     init_T: " << _init_T << endl;
    cerr << "       iter: " << iter << endl;
    cerr << "success num: " << tot_feas << endl;
    cerr << "  rej_ratio: " << float(rej_num)/cnt << endl;
    cerr << "temperature: " << _T << endl;
    _fp.restore(_best_sol);
    _fp.init();
    float cost = norm_cost(_fp.cost());
    uint limit = 100000;
    vector<uint> axis(limit);
    iota(axis.begin(), axis.end(), 1);
    plot_2d<float, uint>(Ts, axis, true, limit);
    plot_2d<float, float>(bests, ax, false, limit);
    _fp.plot();
  }
private:
  float norm_cost(const uint3& cost) const {
    const float r = float(get<2>(cost))/get<1>(cost);
    return (_alpha*get<1>(cost)*get<2>(cost)/_avg_area
         + _beta*get<0>(cost)/_avg_hpwl
         + (1-_alpha-_beta)*(r-_R)*(r-_R)/_avg_r
         + (get<1>(cost) > _W || get<2>(cost) > _H))/2;
  }
  bool feas(const uint3& cost) {
    return (get<1>(cost) <= _W && get<2>(cost) <= _H);
  }
  FLOOR_PLAN<ID, LEN>& _fp;
  typename FLOOR_PLAN<ID, LEN>::TREE _best_sol;
  float _best_cost;
  const ID _Nblcks, _N;
  const uint _W, _H;
  const float _alpha_base, _R;
  float _alpha, _beta, _init_T, _avg_hpwl, _avg_area, _avg_r;
  ID _N_feas;
  list<bool> _recs;
};

int main(int argc, char** argv) {
  assert(argc <= 6);
  srand(time(NULL));
  ifstream fblcks(argv[2], ifstream::in);
  ifstream fnets(argv[3], ifstream::in);
  uint Nnets, Nblcks, W, H;
  string ign;
  fnets >> ign >> Nnets;
  fblcks >> ign >> W >> H;
  fblcks >> ign >> Nblcks;
  auto fp = FLOOR_PLAN<ushort, uint>(fnets, fblcks, argv, Nnets, Nblcks, W, H);
  float P = 0.95, alpha_base = 0.5, beta = 0.0, R = float(H)/W;
  auto sa = SA<ushort, uint>(fp, Nblcks, W, H, R, P, alpha_base, beta); 
  uint k = max(7u, Nblcks/5), rnd = 10*Nblcks;
  float c = 150;//max(90-2*int(Nblcks), 10);
  sa.run(k, rnd, c);
  ofstream outs(argv[4], ifstream::out);
  fp.output(outs);
  outs.close();
}
