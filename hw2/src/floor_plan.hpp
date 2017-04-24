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
      _tree(Nblcks), _W(W), _H(H), _alpha(stof(argv[1])), _has_init(false),
      _rot_prob(0.3), _del_and_ins_prob(0.5) {
    //reading input.block
    string ign;
    fblcks >> ign >> _Ntrmns;
    _blcks.resize(1, {0, 0, 0, "NULL"});
    read_in(_blcks, fblcks, _blcks_id, _Nblcks, 1);
    read_in(_blcks, fblcks, _blcks_id, _Ntrmns, 2);
    fblcks.close();
    int cnt = 0;
    for(ID i = 1; i<=Nblcks; ++i) {
      int min_wh = min(_W, _H);
      if(max(_blcks[i]._w, _blcks[i]._h) > min_wh) {
        ++cnt;
        if(_blcks[i]._w > _W || _blcks[i]._h > _H) {
          _blcks[i]._rot = true;
          swap(_blcks[i]._w, _blcks[i]._h);
          _tree.set_rot(i, true);
        }
      } else _rotable.push_back(i);
    }
    _rot_prob *= (1-float(cnt)/Nblcks);
    //reading input.nets
    _nets.reserve(_Nnets);
    for(int i = 1; i<=_Nnets; ++i) {
      int deg; fnets >> ign >> deg;
      _nets.emplace_back(i);
      auto& net = _nets.back();
      net._blcks.reserve(deg);
      for(int j = 0; j<deg; ++j) {
        string name; fnets >> name;
        ID bid = _blcks_id[name];
        net._blcks.push_back(bid);
        if(bid > _Nblcks) net.update(_blcks[bid]._x, _blcks[bid]._y);
      }
    }
    fnets.close();
  }
  void init() {
    if(_has_init) return;
    _tree.init(_blcks);
    _has_init = true;
  }
  int3 cost(bool get_area = true, bool get_hpwl = true) const {
    assert(get_area|get_hpwl);
    assert(_has_init);

    LEN MAX_X = 0, MAX_Y = 0;
    if(get_area) {
      for(ID i = 1; i<=_Nblcks; ++i) {
        MAX_X = max(MAX_X, LEN(_blcks[i]._x+_blcks[i]._w));
        MAX_Y = max(MAX_Y, LEN(_blcks[i]._y+_blcks[i]._h));
      }
      if(!get_hpwl) return {1, MAX_X, MAX_Y};
    }
    int hpwl = 0;
    for(auto& net:_nets) {
      LEN min_x = net._mnx, min_y = net._mny; 
      LEN max_x = net._mxx, max_y = net._mxy; 
      for(auto& id:net._blcks) if(id <= _Nblcks) {
        const LEN& x = _blcks[id]._x + _blcks[id]._w/2;
        const LEN& y = _blcks[id]._y + _blcks[id]._h/2;
        min_x = min(min_x, x);
        max_x = max(max_x, x);
        min_y = min(min_y, y);
        max_y = max(max_y, y);
      }
      hpwl += ((max_x-min_x) + (max_y-min_y));
    }
    return {hpwl, MAX_X, MAX_Y};
  }
  void plot() const {
    assert(_has_init);
    Gnuplot gp;
    gp << "set xrange [0:" << _W*4 << "]\nset yrange [0:" << _H*4 << "]\n";
    for(ID i = 1; i<=_Nblcks; ++i) {
      const BLOCK& blck = _blcks[i];
      gp << "set object " << int(i) << " rect from " << int(blck._x) 
         << "," << int(blck._y) << " to " << int(blck._x+blck._w) << "," 
         << int(blck._y+blck._h) << " fc rgb \"gray\" front\n";
      gp << "set label \"" << int(blck._id) << "\" at " << int(blck._x+blck._w/2)
         << "," << int(blck._y+blck._h/2) << " front\n";
    }
    _tree.plot(gp, _blcks);
    gp << "set object " << _Nblcks+1 << " rect from 0,0 to " << _W << "," << _H
       << " fc rgb \"red\" back\n";
    gp << "set nokey\n";
    gp << "set size ratio -1\n";
    gp << "set style line 1 lc rgb \'#0060ad\' pt 6\n";
    gp << "plot '-' w p ls 1\n";
    for(ID i = _Nblcks+1; i<_blcks.size(); ++i)
      gp << _blcks[i]._x << " " << _blcks[i]._y << "\n";
    gp << "e\npause -1\n";
  }
  void output(ostream& out) const {
    assert(_has_init);
    int width, height, hpwl; tie(hpwl, width, height) = cost();
    out << _alpha*width*height + (1-_alpha)*hpwl << "\n";
    out << hpwl << "\n";
    out << width*height << "\n";
    out << width << " " << height << "\n";
    out << double(clock()-start_time)/CLOCKS_PER_SEC << "\n";
    for(ID i = 1; i<=_Nblcks; ++i) {
      out << _blcks[i]._name << " " << int(_blcks[i]._x) << " "
          << int(_blcks[i]._y) << " " << int(_blcks[i]._x+_blcks[i]._w) << " "
          << int(_blcks[i]._y+_blcks[i]._h) << "\n";
    }
  }
  void perturb() {
    float p1 = randf(), p2 = randf(), p3 = randf();
    if(p1 < _rot_prob) rotate();
    else if(p2 < _del_and_ins_prob) del_and_ins();
    else swap_two_nodes();
    _has_init = false;
  }
  void restore(const TREE& tree) {
    _tree = tree;
    for(ID i = 1; i<=_Nblcks; ++i) if(_blcks[i]._rot ^ _tree.rot(i)) {
      swap(_blcks[i]._w, _blcks[i]._h);
      _blcks[i]._rot = _tree.rot(i);
    }
    _has_init = false;
  }
  const float _R() { return float(_H)/_W; }
  TREE get_tree() { return _tree; }
private:
  void read_in(vector<BLOCK>& vec, ifstream& ifs, map<string, ID>& m,
               ID num, uchar len) {
    const float R = _R();
    for(ID i = 1; i<=num; ++i) {
      string name; LEN w, h;
      ifs >> name;
      if(len == 2) { string ign; ifs >> ign; }
      ifs >> w >> h;
      m[name] = i;
      if(len == 1) vec.emplace_back(i, w, h, name);
      else vec.emplace_back(i, 0, 0, "t"+to_string(i), w, h);
    }
  }
  void rotate() {
    ID id = _rotable[(rand()%_rotable.size())];
    _blcks[id]._rot = !_blcks[id]._rot;
    swap(_blcks[id]._w, _blcks[id]._h);
    _tree.rotate(id);
  }
  bool del_and_ins() {
    ID id = (rand()%_Nblcks)+1;
    _tree.del_from_tree(id);
    ID par = (rand()%_Nblcks)+1;
    bool left = randb(), do_swap = randb();
    while(id == par) par = (rand()%_Nblcks)+1;
    _tree.ins_to_tree(par, id, left, do_swap);
    return true;
  }
  void swap_two_nodes() {
    ID id1 = rand()%_Nblcks;
    ID id2 = (id1+((rand()%(_Nblcks-1))+1))%_Nblcks;
    _tree.swap_two_nodes(id1+1, id2+1);
  }
  void swap_children() {
    _tree.swap_children();
  }
  float _alpha, _rot_prob, _del_and_ins_prob;
  int _W, _H, _Nblcks, _Ntrmns, _Nnets;
  string _out_rpt;
  vector<BLOCK> _blcks;
  vector<NET> _nets;
  TREE _tree;
  map<string, ID> _blcks_id;
  bool _has_init;
  vector<ID> _rotable;
  vector<bool> _bads;
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
  void del_from_tree(ID id) {
    if(_nodes[id]._l && _nodes[id]._r) {
      while(_nodes[id]._l && _nodes[id]._r) {
        ID par = id;
        id = (randb()?_nodes[id]._l:_nodes[id]._r);
        swap_near(par, id);
        id = par;
      }
      del_from_tree(id);
    } else if(_nodes[id]._l) {
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
      } 
    } else {
      if(id == _nodes[_nodes[id]._par]._l) _nodes[_nodes[id]._par]._l = 0;
      else if(id == _nodes[_nodes[id]._par]._r) _nodes[_nodes[id]._par]._r = 0;
    }
    _nodes[id]._par = 0;
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
    //if(do_swap) swap(_nodes[id]._l, _nodes[id]._r);
  }
  void swap_children() {
    ID id = rand()%(_nodes.size()-1);
    while(!(_nodes[id]._l+_nodes[id]._r)) id = rand()%(_nodes.size()-1);
    swap(_nodes[id]._l, _nodes[id]._r);
  }
  void print() {
    ID rt = _nodes[0]._par;
    cerr << "root " << int(rt) << "\n";
    for(ID i = 1; i<_nodes.size(); ++i) 
      cerr << _nodes[i]._par << " " << _nodes[i]._l << " " << _nodes[i]._r
           << " " << _nodes[i]._rot << "\n";
  }
  void plot(Gnuplot& gp, const vector<BLOCK>& blcks) const {
    ID cnt = 1;
    for(ID i = 1; i<_nodes.size(); ++i) {
      if(_nodes[i]._l) {
        const ID& l = _nodes[i]._l;
        gp << "set arrow " << int(cnt++) << " from " << blcks[i]._x+blcks[i]._w/2
           << "," << blcks[i]._y+blcks[i]._h/2 << " to "
           << blcks[l]._x+blcks[l]._w/2 << "," << blcks[l]._y+blcks[l]._h/2
           << " nohead front\n";
      }
      if(_nodes[i]._r) {
        const ID& r = _nodes[i]._r;
        gp << "set arrow " << int(cnt++) << " from " << blcks[i]._x+blcks[i]._w/2
           << "," << blcks[i]._y+blcks[i]._h/2 << " to "
           << blcks[r]._x+blcks[r]._w/2 << "," << blcks[r]._y+blcks[r]._h/2
           << " nohead front\n";
      }
    }
  }
  bool rot(ID id) const { return _nodes[id]._rot; }
  void set_rot(ID id, bool rot) { _nodes[id]._rot = rot; }
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
  }
  void swap_near(ID par, ID id) {
    if(par == _nodes[0]._par) _nodes[0]._par = id;
    else if(par == _nodes[_nodes[par]._par]._l) _nodes[_nodes[par]._par]._l = id;
    else if(par == _nodes[_nodes[par]._par]._r) _nodes[_nodes[par]._par]._r = id;
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
    }
    _nodes[id]._par = _nodes[par]._par;
    _nodes[par]._par = id;
  }
  vector<NODE> _nodes;
};
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::BLOCK {
  BLOCK(ID id, LEN w, LEN h, const string& name, LEN x = 0, LEN y = 0, 
        bool rot = false)
    : _id(id), _w(w), _h(h), _name(name), _x(x), _y(y), _rot(rot) {};
  ID _id;
  LEN _w, _h, _x, _y;
  string _name;
  bool _rot;
};
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::NET {
  NET(ID id) : _id(id), _mxx(0), _mxy(0), 
               _mnx(1<<(sizeof(LEN)*8-1)), _mny(1<<(sizeof(LEN)*8-1)) {};
  void update(LEN x, LEN y) {
    _mxx = max(_mxx, x);
    _mxy = max(_mxy, y);
    _mnx = min(_mnx, x);
    _mny = min(_mny, y);
  }
  ID _id;
  LEN _mxx, _mxy, _mnx, _mny;
  vector<ID> _blcks;
};
template<typename ID> struct FLOOR_PLAN<ID, LEN>::NODE {
  NODE() : x(0) {};
  ID _l() { return (x&(((1<<10)-1)<<1)); }
  ID _r() { return (x&(((1<<10)-1)<<11)); }
  ID _r() { return (x&(((1<<10)-1)<<21)); }
  bool _rot() { return x&1; }
  int x;
};
/*
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::NODE {
  NODE(ID id = 0, ID l = 0, ID r = 0, ID par = 0, bool rot = false) : 
       _l(l), _r(r), _par(par), _rot(rot) {};
  ID _l, _r, _par;
  bool _rot;
};
*/
