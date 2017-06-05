#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <set>
#include <tuple>
#include <vector>

using namespace std;

void plot(FILE* f, const vector<int>& Xs, const vector<int>& Ys,
          const vector<tuple<int, int, int>>& edges) {
  fprintf(f, "set size ratio -1\nset nokey\n");
  fprintf(f, "set style line 1 lc rgb '#0060ad' pt 6\n");
  for(uint i = 0; i < edges.size(); ++i)
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", Xs[get<1>(edges[i])],
            Ys[get<1>(edges[i])], Xs[get<2>(edges[i])], Ys[get<2>(edges[i])]);
  //for(uint i = 0; i < Xs.size(); ++i)
  //  fprintf(f, "set label \"(%d,%d)\" at %d,%d\n", 
  //          Xs[i], Ys[i], Xs[i]+1, Ys[i]);
  fprintf(f, "plot '-' pt 7 ps 1.3\n");
  for(uint i = 0; i < Xs.size(); ++i)
    fprintf(f, "%d %d\n", Xs[i], Ys[i]);
  fprintf(f, "e\npause -1\n");
}
inline int dist(const int& x0, const int& y0, const int& x1, const int& y1) {
  return abs(x0 - x1) + abs(y0 - y1);
}
template<typename Set_Comp, typename Val_Comp>
inline void span(const vector<int>& Xs, const vector<int>& Ys,
                 const vector<int>& ord, const vector<int>& X_Y,
                 Set_Comp set_comp, Val_Comp val_comp,
                 vector<tuple<int, int, int>>& edges,
                 vector<vector<int>>& adj, bool upp) {
  set<int, decltype(set_comp)> R(set_comp);
  for(auto& p : ord) {
    if(!R.empty()) {
      auto head = (upp ? R.upper_bound(p) : R.lower_bound(p));
      while(head != R.end() && val_comp(X_Y[*head], X_Y[p])) {
        edges.emplace_back(dist(Xs[*head], Ys[*head], Xs[p], Ys[p]), *head, p);
        adj[*head].push_back(p);
        adj[p].push_back(*head);
        head = R.erase(head);
      }
    }
    R.insert(p);
  }
}
struct disjoint_set1 {
  disjoint_set1(int N) : par(N, -1) {}
  int root(int a) { return par[a] < 0 ? a : par[a] = root(par[a]); }
  int unite(int a, int b) {
    a = root(a);
    b = root(b);
    if(a == b) return a;
    if(a > b) swap(a, b);
    par[b] += par[a];
    par[a] = b;
    return b;
  }
  bool same(int a, int b) { return root(a) == root(b); }
  int size(int a) { return -par[a]; }
  vector<int> par;
};
struct disjoint_set2 {
  disjoint_set2(int N) : par(N, -1) {}
  int root(int a) { return par[a] < 0 ? a : par[a] = root(par[a]); }
  void unite(int a, int b) {
    a = root(a);
    b = root(b);
    if(a == b) return;
    if(par[a] < par[b]) swap(a, b);
    par[b] += par[a];
    par[a] = b;
  }
  bool same(int a, int b) { return root(a) == root(b); }
  int size(int a) { return -par[a]; }
  vector<int> par;
};
inline void tarjan(const int& u, const int& numPins, disjoint_set2& ds2, 
            vector<vector<tuple<int, int, int>>>& Qs, 
            const vector<pair<int, int>>& T, vector<int>& ancs, 
            vector<bool>& colors, vector<tuple<int, int, int, int>>& ans) {
  ancs[u] = u;
  if(u >= numPins) {
    tarjan(T[u - numPins].first,  numPins, ds2, Qs, T, ancs, colors, ans);
    ds2.unite(u, T[u - numPins].first);
    ancs[ds2.root(u)] = u;
    tarjan(T[u - numPins].second, numPins, ds2, Qs, T, ancs, colors, ans);
    ds2.unite(u, T[u - numPins].second);
    ancs[ds2.root(u)] = u;
  }
  colors[u] = true;
  if(u < numPins) for(auto &Q : Qs[u])
    ans.emplace_back(tuple_cat(Q, tie(ancs[ds2.root(get<1>(Q))])));
}
inline int calc_cost(int x0, int y0, int x1, int y1, int x2, int y2) {
  int rtn = 0;
  int mxx = max(x1, x2), mnx = min(x1, x2);
  int mxy = max(y1, y2), mny = min(y1, y2);
  if(x0 > mxx) rtn += x0 - mxx;
  else if(x0 < mnx) rtn += mnx - x0;
  if(y0 > mxy) rtn += y0 - mxy;
  else if(y0 < mny) rtn += mny - y0;
  return rtn;
}
inline int calc_gain(const vector<int>& Xs, const vector<int>& Ys,
                     const int& numPins, const vector<pair<int, int>>& MBT,
                     const tuple<int, int, int, int>& t) {
  const int &w = get<0>(t), &u0 = get<0>(t), &v0 = get<2>(t), &x = get<3>(t);
  const int &u1 = MBT[x - numPins].first, &v1 = MBT[x - numPins].second;
  return dist(Xs[u1], Ys[u1], Xs[v1], Ys[v1]) -
         calc_cost(Xs[w], Ys[w], Xs[u0], Ys[u0], Xs[v0], Ys[v0]);
}
int main(int argc, char** argv) {
  assert(argc <= 4);
  //parsing
  int MINX, MINY, MAXX, MAXY;
  FILE *fin = fopen(argv[1] ,"r");
  fscanf(fin, "Boundary = (%d,%d), (%d,%d)\n", &MINX, &MINY, &MAXX, &MAXY);
  int numPins;
  fscanf(fin, "NumPins = %d\n", &numPins);
  vector<int> Xs(numPins), Ys(numPins), X_pls_Y(numPins), X_mns_Y(numPins);
  for(int i = 0; i < numPins; ++i) {
    fscanf(fin, "PIN %*s (%d,%d)\n", &Xs[i], &Ys[i]);
    X_pls_Y[i] = Xs[i] + Ys[i];
    X_mns_Y[i] = Xs[i] - Ys[i];
  }
  //construct spanning graph
  vector<tuple<int, int, int>> edges;
  vector<vector<int>> adj(numPins);
  edges.reserve(numPins * 5);
  vector<int> ord_pls(numPins), ord_mns(numPins);
  iota(ord_pls.begin(), ord_pls.end(), 0);
  iota(ord_mns.begin(), ord_mns.end(), 0);
  sort(ord_pls.begin(), ord_pls.end(), [&](int& p1, int& p2) {
         return X_pls_Y[p1] < X_pls_Y[p2]; });
  sort(ord_mns.begin(), ord_mns.end(), [&](int& p1, int& p2) {
         return X_mns_Y[p1] < X_mns_Y[p2]; });
  auto grtr_x = [&](const int& p1, const int& p2) { return Xs[p1] > Xs[p2]; };
  auto grtr_y = [&](const int& p1, const int& p2) { return Ys[p1] > Ys[p2]; };
  auto less_y = [&](const int& p1, const int& p2) { return Ys[p1] < Ys[p2]; };
  span(Xs, Ys, ord_pls, X_mns_Y, grtr_x, greater<int>(), edges, adj, false);
  span(Xs, Ys, ord_pls, X_mns_Y, grtr_y, less_equal<int>(), edges, adj, true);
  span(Xs, Ys, ord_mns, X_pls_Y, less_y, less<int>(), edges, adj, false);
  span(Xs, Ys, ord_mns, X_pls_Y, grtr_x, greater_equal<int>(), edges, adj, true);
  //construct steiner-tree
  disjoint_set1 ds1(2*numPins - 1); 
  sort(edges.begin(), edges.end(), 
       [](const tuple<int, int, int>& t1, const tuple<int, int, int>& t2) {
         return t1 < t2; });
  int edge_id = 0;
  vector<vector<int>> T;
  vector<vector<tuple<int, int, int>>> Qs(numPins);
  Qs.reserve(2 * numPins);
  vector<pair<int, int>> MBT(numPins - 1);
  for(auto& edge : edges) {
    int &u = get<1>(edge), &v = get<2>(edge);
    int ru = ds1.root(u), rv = ds1.root(v);
    if(ru != rv) {
      T[u].push_back(v);
      T[v].push_back(u);
      for(int& w : adj[u]) if(w != v) {
        if(ds1.same(ru, w)) 
          Qs[w].emplace_back(u, u, v), Qs[u].emplace_back(w, u, v);
        else if(ds1.same(rv, w)) 
          Qs[w].emplace_back(v, v, u), Qs[v].emplace_back(w, v, u);
      }
      for(int& w : adj[v]) if(w != u) {
        if(ds1.same(ru, w)) 
          Qs[w].emplace_back(u, u, v), Qs[u].emplace_back(w, u, v);
        else if(ds1.same(rv, w)) 
          Qs[w].emplace_back(v, v, u), Qs[v].emplace_back(w, v, u);
      }
      MBT[edge_id] = {ru, rv};
      ds1.unite(edge_id + numPins, ds1.unite(ru, rv));
      ++edge_id;
    }
  }
  disjoint_set2 ds2(2*numPins - 1);
  vector<tuple<int, int, int, int>> ans;
  ans.reserve(2*numPins);
  vector<bool> colors(2*numPins - 1);
  vector<int> ancs(2*numPins - 1);
  tarjan(edge_id + numPins - 1, numPins, ds2, Qs, MBT, ancs, colors, ans);
  sort(ans.begin(), ans.end(),
       [&](const tuple<int, int, int, int>& t1,
           const tuple<int, int, int, int>& t2) {
         return calc_gain(Xs, Ys, numPins, MBT, t1) > 
                calc_gain(Xs, Ys, numPins, MBT, t2);
       });
  int total = 0;
  for(auto & an : ans) {
  }
  if(argc == 4) {
    FILE *fplt = fopen(argv[3], "w");
    plot(fplt, Xs, Ys, edges);
  }
}
