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
  FLOOR_PLAN(ifstream& fnets, ifstream& fblcks, char** argv,
             int Nnets, int Nblcks, int W, int H) 
    : _out_rpt(argv[4]), _Nnets(Nnets), _Nblcks(Nblcks), 
      _tree(Nblcks), _W(W), _H(H) {
    _alpha = stod(argv[1]);
    //reading input.block
    string ign;
    fblcks >> ign >> _Ntrmns;
    _blcks.resize(1, {0, 0, 0, "NULL"});
    read_in(_blcks, fblcks, _blcks_id, _Nblcks, 1);
    read_in(_blcks, fblcks, _blcks_id, _Ntrmns, 2);
    fblcks.close();
    //reading input.nets
    _nets.reserve(_Nnets);
    for(ID i = 1; i<=_Nnets; ++i) {
      int deg; fnets >> ign >> deg;
      _nets.emplace_back(i);
      auto& net = _nets.back();
      net._blcks.reserve(deg);
      for(int j = 0; j<deg; ++j) {
        string name; fnets >> name;
        net._blcks.push_back(_blcks_id[name]);
      }
    }
    fnets.close();
  }
  void init() {
    _tree.init(_blcks);
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
    //cerr << "hpwl = " << (int)hpwl << endl;
    //cerr << "area = " << (int)MAX_X*MAX_Y << endl;
    //cerr << "cost = " << _alpha*MAX_X*MAX_Y + (1-_alpha)*hpwl << endl;
    return {hpwl, MAX_X, MAX_Y};
  }
  void plot() {
    Gnuplot gp;
    gp << "set xrange [0:" << _W*2 << "]\nset yrange [0:" << _H*2 << "]" << endl;
    for(uint i = 1; i<=_Nblcks; ++i) {
      const BLOCK& blck = _blcks[i];
      if(!_tree.exist(i)) {
        //cerr << int(i) << " don't exists!" << endl;
        continue;
      }
      gp << "set object " << int(i) << " rect from " << int(blck._x) 
         << "," << int(blck._y) << " to " << int(blck._x+blck._w) << "," 
         << int(blck._y+blck._h) << "fc lt 2" << endl;
      gp << "set label \"" << int(blck._id) << "\" at " << int(blck._x+blck._w/2)
         << "," << int(blck._y+blck._h/2) << endl;
    }
    gp << "set nokey" << endl;
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
      if(!_tree.exist(i)) continue;
      out << _blcks[i]._name << " " << int(_blcks[i]._x) << " "
          << int(_blcks[i]._y) << " " << int(_blcks[i]._x+_blcks[i]._w) << " "
          << int(_blcks[i]._y+_blcks[i]._h) << endl;
    }
  }
  void rotate() {
    ID id = (rand()%_Nblcks)+1;
    _blcks[id]._rot = !_blcks[id]._rot;
    swap(_blcks[id]._w, _blcks[id]._h);
    _tree.rotate(id);
    //cerr << "rotating block: " << id << endl;
  }
  void del_and_ins() {
    //_tree.print();
    ID id = (rand()%_Nblcks)+1;
    if(!_tree.del_from_tree(id)) return;
    //cerr << "deleting " << int(id) << endl;
    ID par = (rand()%_Nblcks)+1;
    bool left = rand()%2, do_swap = rand()%2;
    while(id == par) par = (rand()%_Nblcks)+1;
    //cerr << "inserting " << int(par) << " " << left << " " << do_swap << endl;
    _tree.ins_to_tree(par, id, left, do_swap);
    //_tree.print();
  }
  void swap_two_nodes() {
    ID id1 = rand()%_Nblcks;
    ID id2 = (id1+((rand()%(_Nblcks-1))+1))%_Nblcks;
    //cerr << "swap " << int(id1+1) << " " << int(id2+1) << endl;
    assert(id1 != id2);
    _tree.swap_two_nodes(id1+1, id2+1);
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
      _nodes[id]._id = id;
      _nodes[id]._par = _tree[i/2];
      if(i*2 <= Nblcks) _nodes[id]._l = _tree[i*2];
      if(i*2+1 <= Nblcks) _nodes[id]._r = _tree[i*2+1];
    }
    _nodes[0]._par = _tree[1];
  };
  void init(vector<BLOCK>& blcks) {
    const ID& root_id = _nodes[0]._par;
    blcks[root_id]._x = blcks[root_id]._y = 0;
    list<ID> cy(1, root_id);
    auto cur = cy.begin();
    dfs(root_id, 0, cy, cur, blcks);
  }
  void rotate(ID id) {
    _nodes[id]._rot = !_nodes[id]._rot;
  }
  void swap_two_nodes(ID id1, ID id2) {
    if(_nodes[id1]._par == id2) swap_near(id2, id1);
    else if(_nodes[id2]._par == id1) swap_near(id1, id2);
    else swap_not_near(id1, id2);
  }
  bool del_from_tree(ID id) {
    if(_nodes[id]._l && _nodes[id]._r) return false;
    if(_nodes[id]._l) {
      if(id == _nodes[0]._par) {
        _nodes[0]._par = _nodes[id]._l;
        _nodes[_nodes[id]._l]._par = 0;
      }
      else {
        _nodes[_nodes[id]._l]._par = _nodes[id]._par;
        if(id == _nodes[_nodes[id]._par]._l)
          _nodes[_nodes[id]._par]._l = _nodes[id]._l;
        else if(id == _nodes[_nodes[id]._par]._r)
          _nodes[_nodes[id]._par]._r = _nodes[id]._l;
        else assert(false);
      } 
    } else if(_nodes[id]._r) {
      if(id == _nodes[0]._par) {
        _nodes[0]._par = _nodes[id]._r;
        _nodes[_nodes[id]._r]._par = 0;
      }
      else {
        _nodes[_nodes[id]._r]._par = _nodes[id]._par;
        if(id == _nodes[_nodes[id]._par]._l)
          _nodes[_nodes[id]._par]._l = _nodes[id]._r;
        else if(id == _nodes[_nodes[id]._par]._r)
          _nodes[_nodes[id]._par]._r = _nodes[id]._r;
        else assert(false);
      } 
    } else {
      if(id == _nodes[_nodes[id]._par]._l) _nodes[_nodes[id]._par]._l = 0;
      else if(id == _nodes[_nodes[id]._par]._r) _nodes[_nodes[id]._par]._r = 0;
    }
    _nodes[id]._par = 0;
    return true;
  }
  void ins_to_tree(ID par, ID id, bool left, bool do_swap) {
    if(left) {
      _nodes[id]._l = _nodes[par]._l;
      _nodes[id]._r = 0;
      if(_nodes[par]._l) _nodes[_nodes[par]._l]._par = id;
      _nodes[par]._l = id;
      _nodes[id]._par = par;
    } else {
      _nodes[id]._r = _nodes[par]._r;
      _nodes[id]._l = 0;
      if(_nodes[par]._r) _nodes[_nodes[par]._r]._par = id;
      _nodes[par]._r = id;
      _nodes[id]._par = par;
    }
    if(do_swap) swap(_nodes[id]._l, _nodes[id]._r);
  }
  void print() {
    ID rt = _nodes[0]._par;
    cerr << "root " << int(rt) << endl;
    for(uint i = 1; i<_nodes.size(); ++i) 
      cerr << _nodes[i]._par << " " << _nodes[i]._l << " " 
           << _nodes[i]._r << endl;
  }
  bool exist(ID id) { return _nodes[id]._par || _nodes[0]._par == id; }
private:
  void dfs(ID id, ID par, list<ID>& cy, auto& cur, vector<BLOCK>& blcks) {
    if(_nodes[id]._l) {
      const ID& p = _nodes[id]._l;
      blcks[p]._x = blcks[id]._x + blcks[id]._w;
      blcks[p]._y = find_max_y(cy, ++cur, blcks, blcks[p]);
      dfs(_nodes[id]._l, id, cy, cur, blcks);
      --cur;
    }
    if(_nodes[id]._r) {
      const ID& p = _nodes[id]._r;
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
  void swap_not_near(ID id1, ID id2) {
    if(_nodes[id1]._par == _nodes[id2]._par) {
      swap(_nodes[_nodes[id1]._par]._l, _nodes[_nodes[id1]._par]._r);
    } else {
      if(id1 == _nodes[0]._par) _nodes[0]._par = id2;
      else if(id2 == _nodes[0]._par) _nodes[0]._par = id1;
      if(_nodes[_nodes[id1]._par]._l == id1) 
        _nodes[_nodes[id1]._par]._l = id2;
      else if(_nodes[_nodes[id1]._par]._r == id1) 
        _nodes[_nodes[id1]._par]._r = id2;
      if(_nodes[_nodes[id2]._par]._l == id2)
        _nodes[_nodes[id2]._par]._l = id1;
      else if(_nodes[_nodes[id2]._par]._r == id2) 
        _nodes[_nodes[id2]._par]._r = id1;
    }
    if(_nodes[id1]._l) _nodes[_nodes[id1]._l]._par = id2;
    if(_nodes[id2]._l) _nodes[_nodes[id2]._l]._par = id1;
    if(_nodes[id1]._r) _nodes[_nodes[id1]._r]._par = id2;
    if(_nodes[id2]._r) _nodes[_nodes[id2]._r]._par = id1;
    swap(_nodes[id1]._par, _nodes[id2]._par);
    swap(_nodes[id1]._l, _nodes[id2]._l);
    swap(_nodes[id1]._r, _nodes[id2]._r);
    swap(_nodes[id1]._rot, _nodes[id2]._rot);
  }
  void swap_near(ID par, ID id) {
    if(par == _nodes[0]._par) _nodes[0]._par = id;
    else if(par == _nodes[_nodes[par]._par]._l) _nodes[_nodes[par]._par]._l = id;
    else if(par == _nodes[_nodes[par]._par]._r) _nodes[_nodes[par]._par]._r = id;
    else assert(false);
    if(_nodes[par]._l == id) {
      _nodes[par]._l = _nodes[id]._l;
      if(_nodes[id]._l) _nodes[_nodes[id]._l]._par = par;
      _nodes[id]._l = par;
      if(_nodes[id]._r) _nodes[_nodes[id]._r]._par = par;
      if(_nodes[par]._r) _nodes[_nodes[par]._r]._par = id;
      swap(_nodes[id]._r, _nodes[par]._r);
    } else if(_nodes[par]._r == id) {
      _nodes[par]._r = _nodes[id]._r;
      if(_nodes[id]._r) _nodes[_nodes[id]._r]._par = par;
      _nodes[id]._r = par;
      if(_nodes[id]._l) _nodes[_nodes[id]._l]._par = par;
      if(_nodes[par]._l) _nodes[_nodes[par]._l]._par = id;
      swap(_nodes[id]._l, _nodes[par]._l);
    } else assert(false);
    _nodes[par]._par = id;
    _nodes[id]._par = par;
  }
  vector<NODE> _nodes;
};
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::BLOCK {
  BLOCK(ID id, LEN w, LEN h, const string& name, bool rot = false, 
        LEN x = 0, LEN y = 0)
    : _id(id), _w(w), _h(h), _name(name), _x(x), _y(y), _rot(rot) {};
  ID _id;
  LEN _w, _h, _x, _y;
  string _name;
  bool _rot;
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
