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

extern const clock_t start_time;

template<typename ID, typename LEN>
class FLOOR_PLAN {
public:
  struct BLOCK;
  struct NET;
  struct NODE;
  class TREE;
  FLOOR_PLAN(ifstream& fnets, ifstream& fblcks, char** argv,
             int Nnets, int Nblcks, int Ntrmns, int W, int H) 
    : _out_rpt(argv[4]), _Nnets(Nnets), _Nblcks(Nblcks), _Ntrmns(Ntrmns),
      _tree(Nblcks), _W(W), _H(H), _alpha(stof(argv[1])), _has_init(false),
      _rot_prob(0.3), _del_and_ins_prob(0.5) {
    //reading input.block
    string ign;
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
          _tree.set_rot(i);
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
      net.do_sort();
    }
    fnets.close();
  }
  void init() {
    if(_has_init) return;
    _tree.init(_blcks);
    _has_init = true;
  }
  int3 cost(bool get_area = true, bool get_hpwl = true) const {
    //assert(get_area|get_hpwl);
    //assert(_has_init);
    LEN MAX_X = 0, MAX_Y = 0;
    if(get_area) {
      for(ID i = 1; i<=_Nblcks; ++i) {
        MAX_X = max(MAX_X, LEN(_blcks[i]._x+_blcks[i]._w));
        MAX_Y = max(MAX_Y, LEN(_blcks[i]._y+_blcks[i]._h));
      }
      if(!get_hpwl) return make_tuple(1, MAX_X, MAX_Y);
    }
    int hpwl = 0;
    for(auto& net:_nets) {
      LEN min_x = (LEN)2*net._mnx, min_y = (LEN)2*net._mny; 
      LEN max_x = (LEN)2*net._mxx, max_y = (LEN)2*net._mxy; 
      if(min_x > 0)
        for(auto& id:net._blcks) {
          if(id <= _Nblcks)
            min_x = min(min_x, LEN(2*_blcks[id]._x + _blcks[id]._w));
          else break;
        }
      if(min_y > 0)
        for(auto& id:net._blcks) {
          if(id <= _Nblcks)
            min_y = min(min_y, LEN(2*_blcks[id]._y + _blcks[id]._h));
          else break;
        }
      for(auto& id:net._blcks) if(id <= _Nblcks) {
        const LEN& x = LEN(2*_blcks[id]._x + _blcks[id]._w);
        const LEN& y = LEN(2*_blcks[id]._y + _blcks[id]._h);
        max_x = max(max_x, x);
        max_y = max(max_y, y);
      }
      hpwl += (max_x-min_x+max_y-min_y);
    }
    return make_tuple(hpwl, MAX_X, MAX_Y);
  }
  void plot() const {
    //assert(_has_init);
    Gnuplot gp;
    gp << "set xrange [0:" << 1.8*_W << "]\nset yrange [0:" << 1.8*_H << "]\n";
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
      gp << _blcks[i]._x << " " << _blcks[i]._y << '\n';
    gp << "e\npause -1\n";
  }
  void output(ostream& out) const {
    //assert(_has_init);
    int width, height, hpwl; tie(hpwl, width, height) = cost();
    out << setprecision(13) << _alpha*width*height + (1-_alpha)*hpwl/2. << '\n';
    out << hpwl/2. << '\n';
    out << width*height << '\n';
    out << width << " " << height << '\n';
    out << double(clock()-start_time)/CLOCKS_PER_SEC << '\n';
    for(ID i = 1; i<=_Nblcks; ++i) {
      out << _blcks[i]._name << " " << int(_blcks[i]._x) << " "
          << int(_blcks[i]._y) << " " << int(_blcks[i]._x+_blcks[i]._w) << " "
          << int(_blcks[i]._y+_blcks[i]._h) << '\n';
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
      ID id = i + (len == 2)*_Nblcks;
      m[name] = id;
      if(len == 1) vec.emplace_back(id, w, h, name);
      else vec.emplace_back(id, 0, 0, name, w, h);
    }
  }
  void rotate() {
    ID id = _rotable[(rand()%_rotable.size())];
    _blcks[id]._rot = !_blcks[id]._rot;
    swap(_blcks[id]._w, _blcks[id]._h);
    _tree.rotate(id);
  }
  void del_and_ins() {
    ID id = (rand()%_Nblcks)+1;
    _tree.del_from_tree(id);
    ID p = (rand()%_Nblcks)+1;
    bool left = randb();
    while(id == p) p = (rand()%_Nblcks)+1;
    _tree.ins_to_tree(p, id, left);
  }
  void swap_two_nodes() {
    ID id1 = rand()%_Nblcks;
    ID id2 = (id1+((rand()%(_Nblcks-1))+1))%_Nblcks;
    _tree.swap_two_nodes(id1+1, id2+1);
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
  TREE() {};
  TREE(ID Nblcks) {
    vector<ID> _tree(Nblcks+1);
    iota(_tree.begin()+1, _tree.end(), 1);
    random_shuffle(_tree.begin()+1, _tree.end());
    _nodes.resize(Nblcks+1);
    for(ID i = 1; i<=Nblcks; ++i) {
      ID& id = _tree[i];
      _nodes[id].s_p(_tree[i/2]);
      if(i*2 <= Nblcks) _nodes[id].s_l(_tree[i*2]);
      if(i*2+1 <= Nblcks) _nodes[id].s_r(_tree[i*2+1]);
    }
    _nodes[0].s_p(_tree[1]);
  };
  void init(vector<BLOCK>& blcks) {
    const ID& root_id = _nodes[0]._p();
    blcks[root_id]._x = blcks[root_id]._y = 0;
    list<ID> cy(1, root_id);
    auto cur = cy.begin();
    dfs(root_id, 0, cy, cur, blcks);
  }
  void rotate(ID id) {
    _nodes[id].s_rot();
  }
  void swap_two_nodes(ID id1, ID id2) {
    if(_nodes[id1]._p() == id2) swap_near(id2, id1);
    else if(_nodes[id2]._p() == id1) swap_near(id1, id2);
    else swap_not_near(id1, id2);
  }
  void del_from_tree(ID id) {
    if(_nodes[id]._l() && _nodes[id]._r()) {
      while(_nodes[id]._l() && _nodes[id]._r()) {
        ID p = id;
        id = (randb()?_nodes[id]._l():_nodes[id]._r());
        swap_near(p, id);
        id = p;
      }
      del_from_tree(id);
    } else if(_nodes[id]._l()) {
      if(id == _nodes[0]._p()) {
        _nodes[0].s_p(_nodes[id]._l());
        _nodes[_nodes[id]._l()].s_p(0);
      }
      else {
        _nodes[_nodes[id]._l()].s_p(_nodes[id]._p());
        if(id == _nodes[_nodes[id]._p()]._l())
          _nodes[_nodes[id]._p()].s_l(_nodes[id]._l());
        else if(id == _nodes[_nodes[id]._p()]._r())
          _nodes[_nodes[id]._p()].s_r(_nodes[id]._l());
      } 
    } else if(_nodes[id]._r()) {
      if(id == _nodes[0]._p()) {
        _nodes[0].s_p(_nodes[id]._r());
        _nodes[_nodes[id]._r()].s_p(0);
      }
      else {
        _nodes[_nodes[id]._r()].s_p(_nodes[id]._p());
        if(id == _nodes[_nodes[id]._p()]._l())
          _nodes[_nodes[id]._p()].s_l(_nodes[id]._r());
        else if(id == _nodes[_nodes[id]._p()]._r())
          _nodes[_nodes[id]._p()].s_r(_nodes[id]._r());
      } 
    } else {
      if(id == _nodes[_nodes[id]._p()]._l())
        _nodes[_nodes[id]._p()].s_l(0);
      else if(id == _nodes[_nodes[id]._p()]._r()) 
        _nodes[_nodes[id]._p()].s_r(0);
    }
    _nodes[id].s_p(0);
  }
  void ins_to_tree(ID p, ID id, bool left) {
    if(left) {
      _nodes[id].s_l(_nodes[p]._l());
      _nodes[id].s_r(0);
      if(_nodes[p]._l()) _nodes[_nodes[p]._l()].s_p(id);
      _nodes[p].s_l(id);
      _nodes[id].s_p(p);
    } else {
      _nodes[id].s_r(_nodes[p]._r());
      _nodes[id].s_l(0);
      if(_nodes[p]._r()) _nodes[_nodes[p]._r()].s_p(id);
      _nodes[p].s_r(id);
      _nodes[id].s_p(p);
    }
  }
  void print() const {
    ID rt = _nodes[0]._p();
    cerr << "root " << int(rt) << '\n';
    for(ID i = 1; i<_nodes.size(); ++i) 
      cerr << int(_nodes[i]._p()) << " " << int(_nodes[i]._l()) << " " 
           << int(_nodes[i]._r()) << " " << bool(_nodes[i]._rot()) << '\n';
  }
  void plot(Gnuplot& gp, const vector<BLOCK>& blcks) const {
    ID cnt = 1;
    for(ID i = 1; i<_nodes.size(); ++i) {
      if(_nodes[i]._l()) {
        const ID& l = _nodes[i]._l();
        gp << "set arrow " << int(cnt++) << " from " <<blcks[i]._x+blcks[i]._w/2
           << "," << blcks[i]._y+blcks[i]._h/2 << " to "
           << blcks[l]._x+blcks[l]._w/2 << "," << blcks[l]._y+blcks[l]._h/2
           << " nohead front\n";
      }
      if(_nodes[i]._r()) {
        const ID& r = _nodes[i]._r();
        gp << "set arrow " << int(cnt++) << " from " <<blcks[i]._x+blcks[i]._w/2
           << "," << blcks[i]._y+blcks[i]._h/2 << " to "
           << blcks[r]._x+blcks[r]._w/2 << "," << blcks[r]._y+blcks[r]._h/2
           << " nohead front\n";
      }
    }
  }
  bool rot(ID id) const { return _nodes[id]._rot(); }
  void set_rot(ID id) { _nodes[id].s_rot(); }
private:
  void dfs(ID id, ID p, list<ID>& cy, typename list<ID>::iterator& cur,
           vector<BLOCK>& blcks) {
    if(_nodes[id]._l()) {
      const ID& p = _nodes[id]._l();
      blcks[p]._x = blcks[id]._x + blcks[id]._w;
      blcks[p]._y = find_max_y(cy, ++cur, blcks, blcks[p]);
      dfs(_nodes[id]._l(), id, cy, cur, blcks);
      --cur;
    }
    if(_nodes[id]._r()) {
      const ID& p = _nodes[id]._r();
      blcks[p]._x = blcks[id]._x;
      blcks[p]._y = find_max_y(cy, cur, blcks, blcks[p]);
      dfs(_nodes[id]._r(), id, cy, cur, blcks);
    }
  }
  LEN find_max_y(list<ID>& l, typename list<ID>::iterator& cur,
                 vector<BLOCK>& blcks, BLOCK& blck) {
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
    if(_nodes[id1]._p() == _nodes[id2]._p()) {
      //swap(_nodes[_nodes[id1]._p()]._l(), _nodes[_nodes[id1]._p()]._r());
      ID tmp = _nodes[_nodes[id1]._p()]._l();
      _nodes[_nodes[id1]._p()].s_l(_nodes[_nodes[id1]._p()]._r());
      _nodes[_nodes[id1]._p()].s_r(tmp);
      //swap(_nodes[id1]._l(), _nodes[id2]._l());
      ID tmpl = _nodes[id1]._l();
      _nodes[id1].s_l(_nodes[id2]._l());
      _nodes[id2].s_l(tmpl);
      //swap(_nodes[id1]._r(), _nodes[id2]._r());
      ID tmpr = _nodes[id1]._r();
      _nodes[id1].s_r(_nodes[id2]._r());
      _nodes[id2].s_r(tmpr);
    } else {
      if(id1 == _nodes[0]._p()) _nodes[0].s_p(id2);
      else if(id2 == _nodes[0]._p()) _nodes[0].s_p(id1);
      if(_nodes[_nodes[id1]._p()]._l() == id1) 
        _nodes[_nodes[id1]._p()].s_l(id2);
      else if(_nodes[_nodes[id1]._p()]._r() == id1) 
        _nodes[_nodes[id1]._p()].s_r(id2);
      if(_nodes[_nodes[id2]._p()]._l() == id2)
        _nodes[_nodes[id2]._p()].s_l(id1);
      else if(_nodes[_nodes[id2]._p()]._r() == id2) 
        _nodes[_nodes[id2]._p()].s_r(id1);
    }
    if(_nodes[id1]._l()) _nodes[_nodes[id1]._l()].s_p(id2);
    if(_nodes[id2]._l()) _nodes[_nodes[id2]._l()].s_p(id1);
    if(_nodes[id1]._r()) _nodes[_nodes[id1]._r()].s_p(id2);
    if(_nodes[id2]._r()) _nodes[_nodes[id2]._r()].s_p(id1);
    //swap(_nodes[id1]._p(), _nodes[id2]._p());
    ID tmpp = _nodes[id1]._p();
    _nodes[id1].s_p(_nodes[id2]._p());
    _nodes[id2].s_p(tmpp);
    //swap(_nodes[id1]._l(), _nodes[id2]._l());
    ID tmpl = _nodes[id1]._l();
    _nodes[id1].s_l(_nodes[id2]._l());
    _nodes[id2].s_l(tmpl);
    //swap(_nodes[id1]._r(), _nodes[id2]._r());
    ID tmpr = _nodes[id1]._r();
    _nodes[id1].s_r(_nodes[id2]._r());
    _nodes[id2].s_r(tmpr);
  }
  void swap_near(ID p, ID id) {
    if(p == _nodes[0]._p()) _nodes[0].s_p(id);
    else if(p == _nodes[_nodes[p]._p()]._l()) 
      _nodes[_nodes[p]._p()].s_l(id);
    else if(p == _nodes[_nodes[p]._p()]._r())
      _nodes[_nodes[p]._p()].s_r(id);
    if(_nodes[p]._l() == id) {
      _nodes[p].s_l(_nodes[id]._l());
      if(_nodes[id]._l()) _nodes[_nodes[id]._l()].s_p(p);
      _nodes[id].s_l(p);
      if(_nodes[id]._r()) _nodes[_nodes[id]._r()].s_p(p);
      if(_nodes[p]._r()) _nodes[_nodes[p]._r()].s_p(id);
      ID tmp = _nodes[id]._r();
      _nodes[id].s_r(_nodes[p]._r());
      _nodes[p].s_r(tmp);
      //swap(_nodes[id]._r(), _nodes[p]._r());
    } else if(_nodes[p]._r() == id) {
      _nodes[p].s_r(_nodes[id]._r());
      if(_nodes[id]._r()) _nodes[_nodes[id]._r()].s_p(p);
      _nodes[id].s_r(p);
      if(_nodes[id]._l()) _nodes[_nodes[id]._l()].s_p(p);
      if(_nodes[p]._l()) _nodes[_nodes[p]._l()].s_p(id);
      //swap(_nodes[id]._l(), _nodes[p]._l());
      ID tmp = _nodes[id]._l();
      _nodes[id].s_l(_nodes[p]._l());
      _nodes[p].s_l(tmp);
    }
    _nodes[id].s_p(_nodes[p]._p());
    _nodes[p].s_p(id);
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
               _mnx(1<<(sizeof(LEN)*8-3)), _mny(1<<(sizeof(LEN)*8-3)) {};
  void update(LEN x, LEN y) {
    _mxx = max(_mxx, x);
    _mxy = max(_mxy, y);
    _mnx = min(_mnx, x);
    _mny = min(_mny, y);
  }
  void do_sort() { sort(_blcks.begin(), _blcks.end()); }
  ID _id;
  LEN _mxx, _mxy, _mnx, _mny;
  vector<ID> _blcks;
};
constexpr uint msk_l = ((1<<10)-1)<<1;
constexpr uint msk_r = ((1<<10)-1)<<11;
constexpr uint msk_p = ((1<<10)-1)<<21;
constexpr uint rmsk_l = ~msk_l; 
constexpr uint rmsk_r = ~msk_r; 
constexpr uint rmsk_p = ~msk_p; 
template<typename ID, typename LEN> struct FLOOR_PLAN<ID, LEN>::NODE {
  NODE() : x(0) {};
  ID _l() const { return (x>>01)&((1<<10)-1); }
  ID _r() const { return (x>>11)&((1<<10)-1); }
  ID _p() const { return (x>>21)&((1<<10)-1); }
  bool _rot() const { return x&1; }
  void s_l(const uint& i) { x = ((x&rmsk_l) | ((i<<01)&msk_l)); }
  void s_r(const uint& i) { x = ((x&rmsk_r) | ((i<<11)&msk_r)); }
  void s_p(const uint& i) { x = ((x&rmsk_p) | ((i<<21)&msk_p)); }
  void s_rot() { x ^= 1; }
  int x;
};
