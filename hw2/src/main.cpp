#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>

using namespace std;

const clock_t start_time = clock();
constexpr float temp_th = 0.001, rej_ratio = 0.99;

inline float randf() { return float(rand())/RAND_MAX; }
inline float randb() { return rand()%2; }

typedef tuple<int, int, int> int3;

#include "floor_plan.hpp"

template<typename T, typename U>
void plot_2d(vector<T>& data, vector<U> axis, bool logscale, int limit) {
  assert(axis.size() >= limit || axis.size() == data.size());
  Gnuplot gp;
  gp << setprecision(20);
  if(logscale) gp << "set logscale y 2\n";
  gp << "set nokey\n";
  gp << "set style line 1 lc rgb \'#0060ad\' pt 6\n";
  for(uint i = 1; i<data.size() && i<limit; ++i)
    gp << "set arrow " << i << " from " << axis[i-1] << "," << data[i-1]
       << " to " << axis[i] << "," << data[i] << " nohead\n";
  gp << "plot '-' w p ls 1\n";
  for(uint i = 0; i<data.size() && i<limit; ++i)
    gp << axis[i] << " " << data[i] << '\n';
  gp << "e\npause -1\n";
}

template<typename ID, typename LEN>
class SA {
public: 
  SA(FLOOR_PLAN<ID, LEN>& fp, char** argv, ID Nblcks, int W, int H, float R, 
     float P, float alpha_base,float beta, bool plot) 
    : _fp(fp), _Nblcks(Nblcks), _N(Nblcks), _R(R), _best_sol(fp.get_tree()), 
      _alpha_base(alpha_base), _alpha(alpha_base), _N_feas(0),
      _beta(beta), _W(W), _H(H), _true_alpha(stof(argv[1])), _plot(plot) {
    _fp.init();
    vector<int3> costs(_N+1);
    costs[0] = _fp.cost();
    _avg_r = float(get<2>(costs[0]))/get<1>(costs[0]);
    _avg_hpwl = get<0>(costs[0])/2.;
    _avg_area = get<1>(costs[0])*get<2>(costs[0]);
    _avg_true = _true_alpha*_avg_area + (1-_true_alpha)*_avg_hpwl;
    for(ID i = 1; i<=_N; ++i) {
      _fp.perturb();
      _fp.init();
      costs[i] = _fp.cost();
      const float area = get<1>(costs[i])*get<2>(costs[i]);
      const float hpwl = get<0>(costs[i])/2.;
      _avg_hpwl += hpwl;
      _avg_area += area;
      _avg_true += _true_alpha*area+(1-_true_alpha)*hpwl;
      const float r = float(get<2>(costs[i]))/get<1>(costs[i]);
      _avg_r += (r-R)*(r-R);
    }
    _fp.restore(_best_sol);
    _avg_hpwl = _avg_hpwl*1.1/(_N+1);
    _avg_area = _avg_area*1.1/(_N+1);
    _avg_r = _avg_r*1.1/(_N+1);
    _avg_true = _avg_true*1.1/(_N+1);
    //_best_cost = norm_cost(costs.back());
    float avg_cost = 0;
    for(ID i = 1; i<=_N; ++i)
      avg_cost += abs(norm_cost(costs[i]) - norm_cost(costs[i-1]));
    _init_T = -avg_cost/_N/logf(P);
  };
  void run(const int k, int rnd, const float c) {
    //vector<float> bests, ax, Ts;
    bests.clear(), ax.clear(), Ts.clear();
    tt = clock();
    _recs.resize(_N, false);
    _N_feas = 0;
    _beta = 0.0;
    int reset_th = 2*_Nblcks, stop_th = 4*_Nblcks;
    _fp.init();
    int iter = 1, tot_feas = 0;
    float _T = _init_T, prv_cost = norm_cost(_fp.cost(_alpha, _beta));
    _best_cost = prv_cost;
    int rej_num = 0, cnt = 1;
    typename FLOOR_PLAN<ID, LEN>::TREE last_sol = _fp.get_tree();
    while(_T > temp_th || float(rej_num) <= rej_ratio*cnt || !tot_feas) {
      if(tot_feas) _beta += 0.01;
      float avg_delta_cost = 0;
      rej_num = 0, cnt = 1;
      for(; cnt<=rnd; ++cnt) {
        _fp.perturb();
        _fp.init();
        int3 costs = _fp.cost(_alpha, _beta);
        float cost = norm_cost(costs);
        float delta_cost = (cost - prv_cost);
        avg_delta_cost += abs(delta_cost);

        if(*_recs.begin()) --_N_feas;
        _recs.pop_front();
        if(feas(costs)) {
          ++_N_feas;
          _recs.push_back(true);
          ++tot_feas;
        } else _recs.push_back(false);
        _alpha = _alpha_base + (1-_alpha_base)*_N_feas/_N;

        if(delta_cost <= 0 || randf() < expf(-delta_cost/_T) || tot_feas == 1) {
          prv_cost = cost;
          last_sol = _fp.get_tree();
          if(feas(costs)) {
            if(cost < _best_cost || tot_feas == 1) {
              _best_sol = _fp.get_tree();
              _best_cost = cost;
              if(_plot) {
                bests.push_back(true_cost(_fp.cost()));
                ax.push_back(float(clock()-tt)/CLOCKS_PER_SEC);
              }
            }
          }
        } else {
          _fp.restore(last_sol);
          ++rej_num;
        }
      }
      ++iter;
      if(_plot) Ts.push_back(_T);
      if(iter <= k) _T = _init_T*avg_delta_cost/cnt/iter/c;
      else _T = _init_T*avg_delta_cost/cnt/iter;
      _fp.init();
      if(!tot_feas) {
        if(iter > reset_th) {
          _T = _init_T;
          iter = 1;
          //cerr << "reseting1...\n";
          reset_th += 1;
          stop_th += 1;
          rnd += 1;
        }
      } else if(iter > stop_th) break;
    }
    _fp.restore(_best_sol);
  }
  pair<float, typename FLOOR_PLAN<ID, LEN>::TREE>
  run2(const int k, int rnd, const float c) {
    float _init_T2 = _init_T / 50;
    int reset_th = 2*_Nblcks, stop_th = 9*_Nblcks, reset_cnt = 0;
    int iter = 1, tot_feas = 0, rej_num = 0, cnt = 1;
    _fp.init();
    float _T = _init_T2, prv_cost = true_cost(_fp.cost(), _avg_true);
    _best_cost = prv_cost;
    typename FLOOR_PLAN<ID, LEN>::TREE last_sol = _best_sol;
    while(_T > temp_th || float(rej_num) <= rej_ratio*cnt || !tot_feas) {
      float avg_delta_cost = 0;
      rej_num = 0, cnt = 1;
      for(; cnt<=rnd; ++cnt) {
        _fp.perturb();
        _fp.init();
        int3 costs = _fp.cost();
        float cost = true_cost(costs, _avg_true);
        float delta_cost = (cost - prv_cost);
        avg_delta_cost += abs(delta_cost);

        if(delta_cost <= 0 || randf() < expf(-delta_cost/_T)) {
          prv_cost = cost;
          last_sol = _fp.get_tree();
          if(feas(costs)) {
            ++tot_feas;
            if(cost < _best_cost) {
              _best_sol = _fp.get_tree();
              _best_cost = cost;
              if(_plot) {
                bests.push_back(_best_cost*_avg_true);
                ax.push_back(float(clock()-tt)/CLOCKS_PER_SEC);
              }
            }
          }
        } else {
          _fp.restore(last_sol);
          ++rej_num;
        }
      }
      ++iter;
      if(_plot) Ts.push_back(_T);
      if(iter <= k) _T = _init_T2*avg_delta_cost/cnt/iter/c;
      else _T = _init_T2*avg_delta_cost/cnt/iter;
      _fp.init();
      if(reset_cnt > _Nblcks/7+1) break;
      if(!tot_feas) {
        if(iter > reset_th) {
          _T = _init_T2;
          iter = 1;
          ++reset_th, ++stop_th, ++rnd, ++reset_cnt;
          //cerr << "reseting2...\n";
        }
      } else if(iter > stop_th) break;
    }
    _fp.restore(_best_sol);
    _fp.init();
    int3 costs = _fp.cost();
    _best_cost = true_cost(costs);
    //float hpwl = get<0>(costs)/2.;
    //int area = get<1>(costs)*get<2>(costs);
    //cerr << "     init_T: " << _init_T << '\n';
    //cerr << "temperature: " << _T << '\n';
    //cerr << "       iter: " << iter << '\n';
    //cerr << "success num: " << tot_feas << '\n';
    //cerr << "  rej_ratio: " << float(rej_num)/cnt << '\n';
    //cerr << "      alpha: " << _alpha << '\n';
    //cerr << "       beta: " << _beta << '\n';
    //cerr << "       hpwl: " << hpwl << '\n';
    //cerr << "       area: " << area << '\n';
    //cerr << " total cost: " << _best_cost << '\n';
    if(_plot) {
      int limit = 100000;
      vector<int> axis(limit);
      iota(axis.begin(), axis.end(), 1);
      plot_2d<float, int>(Ts, axis, true, limit);
      plot_2d<float, float>(bests, ax, false, limit);
    }
    return {_best_cost, _best_sol};
  }
private:
  float norm_cost(const int3& cost) const {
    const float r = float(get<2>(cost))/get<1>(cost);
    return (_alpha*get<1>(cost)*get<2>(cost)/_avg_area
         + _beta*get<0>(cost)/_avg_hpwl/2.
         + (1-_alpha-_beta)*(r-_R)*(r-_R)/_avg_r);
  }
  float true_cost(const int3& cost, const float den = 1) const {
    return (_true_alpha*get<1>(cost)*get<2>(cost) 
            + (1-_true_alpha)*get<0>(cost)/2.) / den;
  }
  bool feas(const int3& cost) {
    return (get<1>(cost) <= _W && get<2>(cost) <= _H);
  }
  FLOOR_PLAN<ID, LEN>& _fp;
  typename FLOOR_PLAN<ID, LEN>::TREE _best_sol;
  float _best_cost;
  const ID _Nblcks, _N;
  const int _W, _H;
  const float _alpha_base, _R, _true_alpha;
  float _alpha, _beta, _init_T, _avg_hpwl, _avg_area, _avg_r, _avg_true;
  ID _N_feas;
  list<bool> _recs;
  bool _plot;
  vector<float> bests, Ts, ax;
  clock_t tt;
};
void my_main(int argc, char** argv) {
  ifstream fblcks(argv[2], ifstream::in);
  ifstream fnets(argv[3], ifstream::in);
  ofstream outs(argv[4], ifstream::out);
  int Nnets, W, H, Nblcks, Ntrmns;
  string ign;
  fnets >> ign >> Nnets;
  fblcks >> ign >> W >> H;
  fblcks >> ign >> Nblcks;
  fblcks >> ign >> Ntrmns;
  float P = 0.9, alpha_base = 0.5, beta = 0.1, R = float(H)/W;
  int k = max(2, Nblcks/11), rnd = 2*Nblcks+20;
  float c = max(100-int(Nblcks), 10), costs[2];
  bool plot = (argc > 5 && !strcmp(argv[5], "--plot"));
  bool use_char = (Nblcks+Ntrmns+2) < CHAR_MAX;
  bool use_short = (max(W, H)<<4) < SHRT_MAX;
  if(use_char && use_short) {
    auto fp =
      FLOOR_PLAN<char,short>(fnets, fblcks, argv, Nnets, Nblcks, Ntrmns, W, H);
    auto sa =
      SA<char, short>(fp, argv, Nblcks, W, H, R, P, alpha_base, beta, plot); 
    FLOOR_PLAN<char, short>::TREE trees[2];
    for(int i = 0; i<2; ++i) {
      sa.run(k, rnd, c);
      tie(costs[i], trees[i]) = sa.run2(k, rnd, c);
    }
    fp.restore(costs[0]<costs[1] ? trees[0] : trees[1]);
    fp.init();
    if(plot) fp.plot();
    fp.output(outs);
  } else if(use_char && !use_short) {
    auto fp =
      FLOOR_PLAN<char, int>(fnets, fblcks, argv, Nnets, Nblcks, Ntrmns, W, H);
    auto sa = 
      SA<char, int>(fp, argv, Nblcks, W, H, R, P, alpha_base, beta, plot); 
    FLOOR_PLAN<char, int>::TREE trees[2];
    for(int i = 0; i<2; ++i) {
      sa.run(k, rnd, c);
      tie(costs[i], trees[i]) = sa.run2(k, rnd, c);
    }
    fp.restore(costs[0]<costs[1] ? trees[0] : trees[1]);
    fp.init();
    if(plot) fp.plot();
    fp.output(outs);
  } else if(!use_char && use_short) {
    auto fp =
      FLOOR_PLAN<short, short>(fnets, fblcks, argv, Nnets, Nblcks, Ntrmns, W, H);
    auto sa = 
      SA<short, short>(fp, argv, Nblcks, W, H, R, P, alpha_base, beta, plot);
    FLOOR_PLAN<short, short>::TREE trees[2];
    for(int i = 0; i<2; ++i) {
      sa.run(k, rnd, c);
      tie(costs[i], trees[i]) = sa.run2(k, rnd, c);
    }
    fp.restore(costs[0]<costs[1] ? trees[0] : trees[1]);
    fp.init();
    if(plot) fp.plot();
    fp.output(outs);
  } else if(!use_char && !use_short) {
    auto fp =
      FLOOR_PLAN<short, int>(fnets, fblcks, argv, Nnets, Nblcks, Ntrmns, W, H);
    auto sa = 
      SA<short, int>(fp, argv, Nblcks, W, H, R, P, alpha_base, beta, plot);
    FLOOR_PLAN<short, int>::TREE trees[2];
    for(int i = 0; i<2; ++i) {
      sa.run(k, rnd, c);
      tie(costs[i], trees[i]) = sa.run2(k, rnd, c);
    }
    fp.restore(costs[0]<costs[1] ? trees[0] : trees[1]);
    fp.init();
    if(plot) fp.plot();
    fp.output(outs);
  }
  outs.close();
}
int main(int argc, char** argv) {
  ios_base::sync_with_stdio(false);
  srand(time(NULL));
  my_main(argc, argv);
}
