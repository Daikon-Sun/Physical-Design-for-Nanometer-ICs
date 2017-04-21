#include <cassert>
#include <list>
#include <map>
#include <numeric>
#include <tuple>
#include <vector>

using namespace std;

#include "../gnuplot-iostream/gnuplot-iostream.h"

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

extern clock_t start_time;

template<typename ID, typename LEN>
class FLOOR_PLAN {
public:
  struct BLOCK;
  struct NET;
  struct NODE;
  class TREE;
  FLOOR_PLAN(ifstream& f_nets, ifstream& f_blcks, char** argv,
             int Nnets, int Nblcks, int W, int H) 
    : _out_rpt(argv[4]), _Nnets(Nnets), _Nblcks(Nblcks), 
      _tree(Nblcks), _W(W), _H(H) {
    _alpha = stod(argv[1]);
    //reading input.block
    string ign;
    f_blcks >> ign >> _Ntrmns;
    _blcks.resize(1, {0, 0, 0, "NULL"});
    read_in(_blcks, f_blcks, _blcks_id, _Nblcks, 1);
    read_in(_blcks, f_blcks, _blcks_id, _Ntrmns, 2);
    f_blcks.close();
    //reading input.nets
    _nets.reserve(_Nnets);
    for(ID i = 1; i<=_Nnets; ++i) {
      int deg; f_nets >> ign >> deg;
      _nets.emplace_back(i);
      auto& net = _nets.back();
      net._blcks.reserve(deg);
      for(int j = 0; j<deg; ++j) {
        string name; f_nets >> name;
        net._blcks.push_back(_blcks_id[name]);
      }
    }
    f_nets.close();
  }
  void init() {
    _tree.init(_blcks);
    //ID p = _tree.rood_id(), cnt = 1;
    //list<ID> cy(1, p);
    //auto cur = cy.begin();
    //NODE<ID> *p = _tree.root(), *pre;

    //_blcks[p->_id]._x = _blcks[p->_id]._y = 0; 

    //vector<uchar> vis(_Nblcks+1);
    //++vis[p->id];

    //while(cnt < _Nblcks) {
    //  pre = p;
    //  if(!_tree[p*2] || vis[_tree[p]]>2 || (!_tree[p*2+1] && vis[_tree[p]]>1)) {
    //    if(p%2 == 0) --cur;
    //    p /= 2;
    //  } else if(vis[_tree[p]] == 2) {
    //    p = p*2+1;
    //    _blcks[_tree[p]]._pos = p;
    //    _blcks[_tree[p]]._x = _blcks[_tree[pre]]._x;
    //    _blcks[_tree[p]]._y = find_max_y(cy, cur, _blcks[_tree[p]]);
    //    ++cnt;
    //  } else {
    //    p *= 2;
    //    _blcks[_tree[p]]._pos = p;
    //    _blcks[_tree[p]]._x = _blcks[_tree[pre]]._x + _blcks[_tree[pre]]._w;
    //    _blcks[_tree[p]]._y = find_max_y(cy, ++cur, _blcks[_tree[p]]);
    //    ++cnt;
    //  }
    //  ++vis[_tree[p]];
    //}
  }
  tuple<int, int, int> cost() {
    int hpwl = 0;
    LEN MAX_X = 0, MAX_Y = 0;
    for(auto& net:_nets) {
      LEN min_x, min_y, max_x, max_y;
      min_x = min_y = (1<<(sizeof(LEN)*8-1));
      max_x = max_y = 0;
      for(auto& id:net._blcks) {
        LEN x = _blcks[id]._x + (id<=_Nblcks ? _blcks[id]._w/2 : 0);
        LEN y = _blcks[id]._y + (id<=_Nblcks ? _blcks[id]._h/2 : 0);
        if(id <= _Nblcks) {
          MAX_X = max(MAX_X, _blcks[id]._x+_blcks[id]._w);
          MAX_Y = max(MAX_Y, _blcks[id]._y+_blcks[id]._h);
        }
        min_x = min(min_x, x);
        max_x = max(min_x, x);
        min_y = min(min_y, y);
        max_y = max(min_y, y);
      }
      hpwl += ((max_x-min_x) + (max_y-min_y));
    }
    cerr << "hpwl = " << (int)hpwl << endl;
    cerr << "area = " << (int)MAX_X*MAX_Y << endl;
    cerr << "cost = " << _alpha*MAX_X*MAX_Y + (1-_alpha)*hpwl << endl;
    return {hpwl, MAX_X, MAX_Y};
  }
  void plot() {
    getchar();
    Gnuplot gp;
    gp << "set xrange [0:" << _W*2 << "]\nset yrange [0:" << _H*2 << "]" << endl;
    for(uint i = 1; i<=_Nblcks; ++i) {
      const BLOCK& blck = _blcks[i];
      gp << "set object " << int(i) << " rect from " << int(blck._x) 
         << "," << int(blck._y) << " to " << int(blck._x+blck._w) << "," 
         << int(blck._y+blck._h) << "fc lt 2" << endl;
      gp << "set label \"" << int(blck._id) << "\" at " << int(blck._x+blck._w/2)
         << "," << int(blck._y+blck._h/2) << endl;
    }
    gp << "set size ratio -1" << endl;
    gp << "plot '-' w p ls 1" << endl;
    gp << "0 0" << endl;
    gp << "pause -1" << endl;
  }
  void output(ostream& out) {
    int width, height, hpwl; tie(hpwl, width, height) = cost();
    out << _alpha*width*height + (1-_alpha)*hpwl << endl;
    out << hpwl << endl;
    out << width*height << endl;
    out << width << " " << height << endl;
    out << double(clock()-start_time)/CLOCKS_PER_SEC << endl;
    for(ID i = 1; i<=_Nblcks; ++i) {
      out << _blcks[i]._name << " " << int(_blcks[i]._x) << " "
          << int(_blcks[i]._y) << " " << int(_blcks[i]._x+_blcks[i]._w) << " "
          << int(_blcks[i]._y+_blcks[i]._h) << endl;
    }
  }
  void rotate() {
    ID id = (rand()%_Nblcks)+1;
    cerr << "rotating block: " << id << endl;
    BLOCK& blck = _blcks[id];
    swap(blck._w, blck._h);
  }
  void del_and_ins() {
    ID id = (rand()%_Nblcks)+1;
    bool left = rand()%2;
    //del_from_tree(id, left);
  }
private:
  void read_in(vector<BLOCK>& vec, ifstream& ifs, map<string, ID>& m,
               ID num, uchar len) {
    for(ID i = 1; i<=num; ++i) {
      vector<string> names(2); LEN w, h;
      for(uchar j = 0; j<len; ++j) ifs >> names[j];
      ifs >> w >> h;
      m[names[0]] = i;
      if(len == 1) vec.emplace_back(i, w, h, names[0]);
      else vec.emplace_back(i, 0, 0, "", w, h);
    }
  }
  //void del_from_tree(ID id, bool left) {
  //  BLOCK& blck = _blcks[id];
  //  int p = blck._pos;
  //  blck._pos = 0;
  //  if(!_tree[p*2] && !_tree[p*2+1]) {
  //    _tree[p] = 0;
  //  } else if(!_tree[p*2] && _tree[p*2+1]) {
  //    _tree[p] = _tree[p*2+1];
  //  } else if(_tree[p*2] && !_tree[p*2+1]) {
  //    _tree[p] = _tree[p*2];
  //  } else {
  //    _tree[p] = (left ? _tree[p*2] : _tree[p*2+1]);
  //  }
  //}
  double _alpha;
  uint _W, _H, _Nblcks, _Ntrmns, _Nnets;
  string _out_rpt;
  vector<BLOCK> _blcks;
  vector<NET> _nets;
  TREE _tree;
  map<string, ID> _blcks_id;
};
template<typename ID, typename LEN> class FLOOR_PLAN<ID, LEN>::TREE {
public:
  TREE(ID Nblcks) {
    vector<ID> _tree(Nblcks+1);
    iota(_tree.begin()+1, _tree.end(), 1);
    random_shuffle(_tree.begin()+1, _tree.end());
    _nodes.resize(Nblcks+1);
    for(ID i = 1; i<=Nblcks; ++i) {
      ID& id = _tree[i];
      _nodes[id]._id = i;
      _nodes[id]._par = _tree[id/2];
      if(i*2 <= Nblcks) _nodes[id]._l = _tree[i*2];
      if(i*2+1 <= Nblcks) _nodes[id]._r = _tree[i*2+1];
    }
    _nodes[0]._par = _tree[1];
  };
  //NODE* root() { return &_nodes[root_id()]; }
  //ID root_id() { return _nodes[0]._par; }
  void init(vector<BLOCK>& blcks) {
    const ID& root_id = _nodes[0]._par;
    list<ID> cy(1, root_id);
    auto cur = cy.begin();
    dfs(root_id, 0, cy, cur, blcks);
  }
  //const NODE& root() { return _blcks[ root_id() ]; }
  //const NODE& operator [] (ID i) { return _blcks[i]; }
private:
  void dfs(ID id, ID par, list<ID>& cy, auto& cur, vector<BLOCK>& blcks) {
    if(_nodes[id]._l) {
      const ID& p = _nodes[id]._l;
      blcks[p]._id = p;
      blcks[p]._x = blcks[id]._x + blcks[id]._w;
      blcks[p]._y = find_max_y(cy, ++cur, blcks, blcks[p]);
      dfs(_nodes[id]._l, id, cy, cur, blcks);
      --cur;
    }
    if(_nodes[id]._r) {
      const ID& p = _nodes[id]._r;
      blcks[p]._id = p;
      blcks[p]._x = blcks[id]._x;
      blcks[p]._y = find_max_y(cy, cur, blcks, blcks[p]);
      dfs(_nodes[id]._r, id, cy, cur, blcks);
    }
  }
  LEN find_max_y(list<ID>& l, auto& cur, vector<BLOCK>& blcks, BLOCK& blck) {
    LEN y = 0;
    auto it = cur, rit = cur, mit = cur;
    while(it != l.end() && blcks[*it]._x < blck._x+blck._w) {
      if(blcks[*it]._y + blcks[*it]._h > y) {
        y = blcks[*it]._y + blcks[*it]._h;
        mit = it;
      }
      if(blcks[*rit]._x+blcks[*rit]._w <= blck._x+blck._w) ++rit;
      ++it;
    }
    cur = l.erase(cur, rit);
    cur = l.insert(cur, blck._id);
    return y;
  }
  vector<NODE> _nodes;
};
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::BLOCK {
  BLOCK(ID id, LEN w, LEN h, const string& name, LEN x = 0, LEN y = 0)
    : _id(id), _w(w), _h(h), _name(name), _x(x), _y(y) {};
  ID _id;
  LEN _w, _h, _x, _y;
  string _name;
};
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::NET {
  NET(ID id) : _id(id) {};
  ID _id;
  vector<ID> _blcks;
};
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::NODE {
  NODE(ID id = 0, ID l = 0, ID r = 0, ID par = 0, bool rot = false) : 
       _id(id), _l(l), _r(r), _par(par), _rot(rot) {};
  ID _id, _l, _r, _par;
  bool _rot;
};