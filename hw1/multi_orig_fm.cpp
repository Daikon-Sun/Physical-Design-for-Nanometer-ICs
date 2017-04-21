#include<algorithm>
#include<cassert>
#include<climits>
#include<cstdlib>
#include<cstdio>
#include<ctime>
#include<iostream>
#include<list>
#include<set>
#include<utility>
#include<vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

using namespace std;

double r; //balance factor
int tot_cell; //total cell count
int mx_cnt = 0; //max pin count for cells
int M = 0; //max gain pointer
int iter = 0; //iteration count
constexpr unsigned total_sec = 3000; //time-limit of this program in second

vector< list<int> > gain_list;
int A, B;

class Cell {
public:
  Cell() : _gain(0), _inA(false), _isFree(true), _it(0) {};
  void add_net(int& nid) { return _nets.push_back(nid); }
  void change_set() { _inA = !_inA; }
  void change_state() { _isFree = !_isFree; }
  bool empty() { return _nets.empty(); }
  bool From() { return _inA; }
  bool inA() { return _inA; }
  bool is_free() { return _isFree; }
  void lock() { _isFree = false; }
  void set_pos(auto& it) {_it = it; }
  void set_gain(int g) { _gain = g; M = max(M, g); }
  int size() { return _nets.size(); }
  bool To() { return !_inA; }
  void unlock() { _isFree = true; }
  void change_gain(int cid, int diff) { 
    gain_list[_gain].erase(_it);
    _gain += diff;
    M = max(M, _gain);
    list<int>& gl = gain_list[_gain];
    auto nit = gl.insert(gl.begin(), cid);
    set_pos(nit);
  }
  const vector<int>& nets() const { return _nets; }
private:
  int _gain;
  bool _inA, _isFree;
  vector<int> _nets;
  list<int>::iterator _it;
};

class Net {
public:
  Net() : _Acnt(0) {};
  int size() { return _cells.size(); }
  bool empty() { return _cells.empty(); }
  void add_cell(int cid) { _cells.push_back(cid); }
  void change_Acnt(bool inc) { inc?(++_Acnt):(--_Acnt);}
  int get_cnt(bool inA) { return inA?_Acnt:((int)_cells.size()-_Acnt); }
  bool cutted() { return (_Acnt!=(int)_cells.size()) && _Acnt; }
  const vector<int>& cells() const { return _cells; }
private:
  int _Acnt;
  vector<int> _cells;
};

vector<Cell> cell_list;
vector<Net> net_list;

inline bool is_good(int cid) {
  if( cell_list[cid].inA() )
    return A-1>=(1-r)*tot_cell/2 && B+1<=(1+r)*tot_cell/2;
  else return B-1>=(1-r)*tot_cell/2 && A+1<=(1+r)*tot_cell/2;
  __builtin_unreachable();
}
inline void parse(const char* filename) {
  for(int i = 0; i<2; ++i) {
    int max_cellID = 0, tot_net = 0;
    FILE* f = fopen(filename, "r");
    (void)fscanf(f, "%lf%*c", &r);
    while( fscanf(f, "NET") != EOF ) {
      (void)fscanf(f, "%*[^0-9]%*d");
      set<int> cells;
      while(true) {
        char c; (void)fscanf(f, "%*[^;c]%c", &c);
        if( c == ';' ) break;
        int cellID; (void)fscanf(f, "%d", &cellID);
        if(!i) max_cellID = max(max_cellID, cellID);
        else if(!cells.count(cellID)) {
          net_list[tot_net].add_cell(cellID);
          cell_list[cellID].add_net(tot_net);
          cells.insert(cellID);
        }
      }
      getc(f);
      ++tot_net;
    }
    fclose(f);
    if(!i) cell_list.resize(max_cellID+1), net_list.resize(tot_net);
  }
  for(size_t i = 0; i<cell_list.size(); ++i)
    if(!cell_list[i].empty()) ++tot_cell;
}
inline double get_total(int cid, int weight) {
  int rtn = 0, total = 0;
  for(int nid:cell_list[cid].nets()) {
    rtn = max(rtn, net_list[nid].size());
    total += net_list[nid].size();
  }
  return rtn+weight*double(total)/cell_list[cid].size();
}
inline void random_split() {
  if(iter == 1) {
    for(int i = 0, cnt = 0; cnt<tot_cell/2; ++i) {
      if(cell_list[i].empty()) continue;
      ++cnt;
      for(int j:cell_list[i].nets()) if(net_list[j].size() > 1)
        net_list[j].change_Acnt(true);
      cell_list[i].change_set();
    }
  } else if(iter > 1 && iter < 8) {
    cerr << "here" << endl;
    vector< pair<double, int> > cell_size;
    for(int i = (int)cell_list.size()-1; i>=0; --i) {
      if(cell_list[i].empty()) continue;
      cell_size.push_back(make_pair(get_total(i, iter-2), i));
    }
    sort(cell_size.begin(), cell_size.end());
    for(auto& info:cell_size) if(A < tot_cell/2) {
      ++A;
      int cid = info.second;
      for(int j:cell_list[cid].nets()) if(net_list[j].size() > 1)
        net_list[j].change_Acnt(true);
      cell_list[cid].change_set();
    }
  } else if(iter == 8) {
    vector< pair<int, int> > cell_size;
    for(int i = 0; i<(int)cell_list.size(); ++i) if(!cell_list[i].empty())
      cell_size.push_back(make_pair(cell_list[i].size(), i));
    sort(cell_size.begin(), cell_size.end());
    for(auto& info:cell_size) if(A < tot_cell/2) {
      ++A;
      int cid = info.second;
      for(int j:cell_list[cid].nets()) if(net_list[j].size() > 1)
        net_list[j].change_Acnt(true);
      cell_list[cid].change_set();
    }
  } else {
    vector<bool> vis(cell_list.size());
    for(int i = 0; i<tot_cell/2; ++i) {
      int num = rand()%tot_cell+1;
      if(vis[num] || cell_list[num].empty()) {
        --i;
      } else {
        vis[num] = true;
        for(int j:cell_list[num].nets()) if(net_list[j].size() > 1)
          net_list[j].change_Acnt(true);
        cell_list[num].change_set();
      }
    }
  }
  A = tot_cell/2;
  B = tot_cell-A;
}
inline void init() {
  //initialize cell gain
  for(size_t i = 0; i<cell_list.size(); ++i) if( !cell_list[i].empty() ) {
    Cell& cell = cell_list[i];
    int g = mx_cnt;
    for(int j:cell.nets()) if(net_list[j].size() > 1) {
      if( cell.inA() ) {
        if(net_list[j].get_cnt(true) == 1) ++g;
        else if(net_list[j].get_cnt(false) == 0) --g;
      } else {
        if(net_list[j].get_cnt(true) == 0) --g;
        else if(net_list[j].get_cnt(false) == 1) ++g;
      }
    }
    auto it = gain_list[g].insert(gain_list[g].begin(), i);
    cell.set_gain(g);
    cell.set_pos(it);
  }
}
inline void change_all(Net& net, bool inc) {
  for(int cid:net.cells()) if(cell_list[cid].is_free())
    cell_list[cid].change_gain(cid, (inc?1:-1));
}
inline void change_only(Net& net, bool to_from, bool inc) {
  int tmpcnt = 0;
  for(int cid:net.cells()) if(to_from == cell_list[cid].From()) {
    if(cell_list[cid].is_free())
      cell_list[cid].change_gain(cid, (inc?1:-1));
  }
  assert(tmpcnt <= 1);
}
inline void update(Net& net, bool to_from, bool inc) {
  if(net.get_cnt(to_from) == 0) change_all(net, inc);
  else if(net.get_cnt(to_from) == 1) change_only(net, to_from, !inc);
}
inline void update_gain(int cid) {
  Cell& cell = cell_list[cid];
  cell.lock();
  for(int nid:cell.nets()) {
    Net& net = net_list[nid];
    if(net.size() <= 1) continue;
    update(net, cell.To(), true);
    net.change_Acnt(!cell.inA());
    update(net, cell.From(), false);
  }
  cell.change_set(); 
}
inline void update_AB(bool inA) {
  if(inA) --A, ++B;
  else ++A, --B;
}
inline int cutsize() {
  return count_if(net_list.begin(), net_list.end(), [](Net& net) {
    return net.cutted(); });
}
inline void print_set(int set_id, int set_num, FILE*& f, list<int>& l) {
  fprintf(f, "G%d %d\n", set_id, set_num);
  for(auto i:l) fprintf(f, "c%d ", i);
  fprintf(f, ";\n");
}
inline void output(const char* filename) {
  list<int> gA, gB;
  for(size_t i = 0; i<cell_list.size(); ++i) if( !cell_list[i].empty() ) {
    if(cell_list[i].inA()) gA.push_back(i);
    else gB.push_back(i);
  }
  FILE* f = fopen(filename, "w");
  fprintf(f, "Cutsize = %d\n", cutsize());
  print_set(1, A, f, gA);
  print_set(2, B, f, gB);
  fclose(f);
}
int main(const int argc, const char* argv[] ) {
  clock_t t0 = clock();
  assert(argc == 3);
  srand(time(NULL));
  int goodCutSize = INT_MAX;
  while(true) {
    clock_t start_time = clock();
    ++iter;
    tot_cell = r = mx_cnt = M = A = B = 0;
    gain_list.clear(), cell_list.clear(), net_list.clear();
    parse(argv[1]);
    random_split();
    for(auto& cell:cell_list) mx_cnt = max(mx_cnt, cell.size());
    gain_list.resize(2*mx_cnt+1); 
    while(true) {
      init();
      M = 2*mx_cnt;
      list<int> gains, steps;
      int tmpM = 0;
      while(M >= 0) {
        if( (clock()-t0) >= total_sec*CLOCKS_PER_SEC) exit(0);
        bool first_time = false;
        if( gain_list[M].empty() ) --M;
        else {
          list<int>& gl = gain_list[M];
          for(auto it = gl.begin(); it!=gl.end(); ++it) {
            if( (clock()-t0) >= total_sec*CLOCKS_PER_SEC) exit(0);
            Cell& cell = cell_list[*it];
            if( cell.is_free() && is_good(*it) ) {
              steps.push_back(*it);
              gains.push_back(M-mx_cnt);
              update_gain(*it);
              update_AB(!cell.inA());
              gl.erase(it);
              first_time = true;
              break;
            }
          }
          if(first_time) M = max(M, tmpM), tmpM = 0;
          else {
            if(!tmpM) tmpM = max(tmpM, M);
            else --M;
          }
        }
      }
      for(int i = 2*mx_cnt; i>=0; --i) gain_list[i].clear();
      int G = 0, maxG = 0;
      for(int _g:gains) {
        G += _g;
        maxG = max(maxG, G);
      }
      if(!maxG) break;
      auto git=gains.rbegin(), sit=steps.rbegin(); 
      for(; git!=gains.rend(); ++sit, ++git) {
        if(G == maxG) break;
        Cell& cell = cell_list[*sit];
        update_AB(cell.inA());
        for(auto nid:cell.nets()) 
          if(net_list[nid].size() > 1 )
            net_list[nid].change_Acnt(!cell.inA());
        cell.change_set();
        G -= *git;
      }
      for(size_t i = 0; i<cell_list.size(); ++i) cell_list[i].unlock();
    }
    int new_cs = cutsize();
    if(new_cs < goodCutSize) {
      goodCutSize = new_cs;
      output(argv[2]);
    }
  }
}
