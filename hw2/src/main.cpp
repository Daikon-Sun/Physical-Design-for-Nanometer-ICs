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

template<typename T, typename U>
void plot_1d(vector<T>& data, vector<U> axis, bool logscale, uint limit) {
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
      if(get<1>(costs[i]) < _W && get<2>(costs[i]) < _H) {
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
    _best_cost = norm_cost(costs[0]);
    float avg_cost = 0;
    for(ID i = 1; i<=_N; ++i) {
      float delta_cost = norm_cost(costs[i]) - norm_cost(costs[i-1]); 
      avg_cost += abs(delta_cost);
    }
    _init_T = -avg_cost/_N/logf(P);
  };
  void run(const uint k, const uint rnd, const float c) {
    clock_t tt = clock();
    vector<float> Ts(1, _init_T), bests(1, _best_cost), ax(1, 0);
    _fp.init();
    uint iter = 1;
    float _T = 2*_init_T;
    uint rej_num = 0, cnt = 1;
    typename FLOOR_PLAN<ID, LEN>::TREE last_sol = _fp.get_tree();
    while(_T > 0.00005 || iter <= 7*k || float(rej_num) <= 0.99f*cnt) {
      float avg_delta_cost = 0, prv_cost = norm_cost(_fp.cost());
      uint uphill; 
      uphill = rej_num = 0, cnt = 1;
      for(; cnt<=rnd && uphill*2<=rnd; ++cnt) {
        _fp.perturb();
        _fp.init();
        uint3 costs = _fp.cost();
        if(*_recs.begin()) --_N_feas;
        _recs.pop_front();
        if(get<1>(costs) < _W && get<2>(costs) < _H) {
          ++_N_feas;
          _recs.push_back(true);
        } else _recs.push_back(false);
        _alpha = _alpha_base + (1-_alpha_base)*_N_feas/_N;
        float cost = norm_cost(costs);
        float delta_cost = (cost - prv_cost);
        avg_delta_cost += delta_cost;
        if(delta_cost <= 0 || randf() < expf(-delta_cost/_T)+1e-3) {
          prv_cost = cost;
          last_sol = _fp.get_tree();
          if(delta_cost < 0) {
            if(cost < _best_cost) {
              _best_sol = _fp.get_tree();
              _best_cost = cost;
              bests.push_back(_best_cost);
              ax.push_back(float(clock()-tt)/CLOCKS_PER_SEC);
            }
          } else if(delta_cost > 0) ++uphill;
        } else {
          _fp.restore(last_sol);
          ++rej_num;
        }
      }
      ++iter;
      if(iter < k) _T = 2*_init_T*abs(avg_delta_cost)/cnt/iter/c;
      else _T = 2*_init_T*abs(avg_delta_cost)/cnt/iter;
      Ts.push_back(_T);
      _fp.init();
      if(iter > 300*_Nblcks) break;
    }
    cerr << "iter " << iter << endl;
    cerr << rej_num << " " << cnt << endl;
    cerr << "rej_ratio  : " << float(rej_num)/cnt << endl;
    cerr << "temperature: " << _T << endl;
    _fp.restore(_best_sol);
    _fp.init();
    float cost = norm_cost(_fp.cost());
    uint limit = 1000;
    vector<uint> axis(limit);
    iota(axis.begin(), axis.end(), 1);
    plot_1d<float, uint>(Ts, axis, true, limit);
    plot_1d<float, float>(bests, ax, false, limit);
    _fp.plot();
  }
private:
  float norm_cost(const uint3& cost) const {
    const float r = float(get<2>(cost))/get<1>(cost);
    return (_alpha*get<0>(cost)/_avg_hpwl 
         + _beta*get<1>(cost)*get<2>(cost)/_avg_area 
         + (1-_alpha-_beta)*(r-_R)*(r-_R)/_avg_r
         + (get<1>(cost) > _W || get<2>(cost) > _H))/2;
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
  uint k = max(2u, Nblcks/6), rnd = 20*Nblcks;
  float c = max(90-2*int(Nblcks), 10);
  sa.run(k, rnd, c);
  ofstream outs(argv[4], ifstream::out);
  fp.output(outs);
  outs.close();
}
