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
typedef long long LL;

inline void plot_rect_line(FILE *f, int x0, int y0, int x1, int y1) {
  if(x0 == x1 || y0 == y1)
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", x0, y0, x1, y1); 
  else {
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", x0, y0, x0, y1);
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", x0, y1, x1, y1); 
  }
}
inline LL dist(const int& x0, const int& y0, const int& x1, const int& y1) {
  return abs(x0 - x1) + abs(y0 - y1);
}
template<typename U>
inline LL plot(FILE* f, const U& numPins,
               const vector<int>& Xs, const vector<int>& Ys,
               const vector<vector<bool>>& T) {
  LL cost = 0;
  if(f) fprintf(f, "set size ratio -1\nset nokey\n");
  if(f) fprintf(f, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7\n");
  #pragma omp parallel for reduction(+:cost)
  for(uint i = 0; i < T.size(); ++i) for(uint j = i+1; j < T.size(); ++j)
    if(T[i][j]) {
      if(f) plot_rect_line(f, Xs[i], Ys[i], Xs[j], Ys[j]);
      cost += dist(Xs[i], Ys[i], Xs[j], Ys[j]);
    }
  if(f) {
    fprintf(f, "set style line 1 lc rgb 'blue' pt 5\n");
    fprintf(f, "set style line 2 lc rgb 'red' pt 7\n");
    fprintf(f, "plot '-' w p ls 1, '-' w p ls 2\n");
    for(int i = 0; i < numPins; ++i) fprintf(f, "%d %d\n", Xs[i], Ys[i]);
    fprintf(f, "e\n");
    for(uint i = numPins; i < Xs.size(); ++i) fprintf(f, "%d %d\n", Xs[i], Ys[i]);
    fprintf(f, "e\npause -1\n");
  }
  return cost;
}
template<typename Set_Comp, typename Val_Comp, typename U>
inline void span(const vector<int>& Xs, const vector<int>& Ys,
                 const vector<U>& ord, const vector<int>& X_Y,
                 Set_Comp set_comp, Val_Comp val_comp,
                 vector<tuple<int, U, U>>& edges,
                 vector<vector<U>>& adj, bool upp) {
  multiset<int, decltype(set_comp)> R(set_comp);
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
template<typename U> struct disjoint_set1 {
  disjoint_set1(int N) : par(N, -1) {}
  U root(U a) { return par[a] < 0 ? a : par[a] = root(par[a]); }
  U unite(U a, U b) {
    a = root(a);
    b = root(b);
    if(a == b) return a;
    if(a > b) swap(a, b);
    par[b] += par[a];
    par[a] = b;
    return b;
  }
  bool same(U a, U b) { return root(a) == root(b); }
  U size(U a) { return -par[a]; }
  vector<U> par;
};
template<typename U> struct disjoint_set2 {
  disjoint_set2(int N) : par(N, -1) {}
  U root(U a) { return par[a] < 0 ? a : par[a] = root(par[a]); }
  void unite(U a, U b) {
    a = root(a);
    b = root(b);
    if(a == b) return;
    if(par[a] < par[b]) swap(a, b);
    par[b] += par[a];
    par[a] = b;
  }
  bool same(U a, U b) { return root(a) == root(b); }
  U size(U a) { return -par[a]; }
  vector<U> par;
};
template<typename U>
inline void tarjan(const U& u, const U& numPins, disjoint_set2<U>& ds2, 
            vector<vector<tuple<U, U, U>>>& Qs, 
            const vector<pair<U, U>>& MBT, vector<U>& ancs, 
            vector<bool>& colors, vector<tuple<U, U, U, U, int>>& ans) {
  ancs[u] = u;
  if(u >= numPins) {
    tarjan(MBT[u - numPins].first,  numPins, ds2, Qs, MBT, ancs, colors, ans);
    ds2.unite(u, MBT[u - numPins].first);
    ancs[ds2.root(u)] = u;
    tarjan(MBT[u - numPins].second, numPins, ds2, Qs, MBT, ancs, colors, ans);
    ds2.unite(u, MBT[u - numPins].second);
    ancs[ds2.root(u)] = u;
  }
  colors[u] = true;
  if(u < numPins) for(auto &Q : Qs[u]) if(colors[get<0>(Q)]) {
    if(u == get<1>(Q))
      ans.emplace_back(get<0>(Q), get<1>(Q), get<2>(Q),
                       ancs[ds2.root(get<0>(Q))], 0);
    else ans.emplace_back(u, get<1>(Q), get<2>(Q), ancs[ds2.root(get<0>(Q))], 0);
  }
}
inline int calc_cost(int x0, int y0, int x1, int y1, int x2, int y2) {
  int rtn = 0, mxx = max(x1, x2), mnx = min(x1, x2);
  int mxy = max(y1, y2), mny = min(y1, y2);
  if(x0 > mxx) rtn += x0 - mxx;
  else if(x0 < mnx) rtn += mnx - x0;
  if(y0 > mxy) rtn += y0 - mxy;
  else if(y0 < mny) rtn += mny - y0;
  return rtn;
}
template<typename U>
inline int calc_gain(const vector<int>& Xs, const vector<int>& Ys,
                     const U& numPins, const vector<pair<U, U>>& Tedge,
                     const tuple<U, U, U, U, int>& t) {
  const U &w = get<0>(t), &u0 = get<1>(t), &v0 = get<2>(t), &x = get<3>(t);
  //assert(x >= numPins);
  const U &u1 = Tedge[x - numPins].first, &v1 = Tedge[x - numPins].second;
  //assert(u0 < numPins && v0 < numPins && u1 < numPins && v1 < numPins);
  return dist(Xs[u1], Ys[u1], Xs[v1], Ys[v1]) -
         calc_cost(Xs[w], Ys[w], Xs[u0], Ys[u0], Xs[v0], Ys[v0]);
}
inline pair<int, int> gen_point(int x0, int y0, int x1, int y1, int x2, int y2) {
  int mxx = max(x1, x2), mnx = min(x1, x2);
  int mxy = max(y1, y2), mny = min(y1, y2);
  if(x0 <= mxx && x0 >= mnx && y0 <= mxy && y0 >= mny)
    return {INT_MIN, INT_MIN};
  int rtnx, rtny;
  if(x0 < mnx) rtnx = mnx;
  else if(x0 > mxx) rtnx = mxx;
  else rtnx = x0;
  if(y0 < mny) rtny = mny;
  else if(y0 > mxy) rtny = mxy;
  else rtny = y0;
  return {rtnx, rtny};
}
template<typename U>
inline LL steiner_tree(vector<int>& Xs, vector<int>& Ys, vector<int>& X_pls_Y,
                          vector<int>& X_mns_Y, vector<vector<bool>>& T) {
  U numPins = (U)Xs.size();
  //construct spanning graph
  vector<tuple<int, U, U>> edges;
  vector<vector<U>> adj(numPins);
  edges.reserve(3 * numPins);
  vector<U> ord_pls(numPins), ord_mns(numPins);
  iota(ord_pls.begin(), ord_pls.end(), 0);
  iota(ord_mns.begin(), ord_mns.end(), 0);
  sort(ord_pls.begin(), ord_pls.end(), [&](U& p1, U& p2) {
         return X_pls_Y[p1] < X_pls_Y[p2]; });
  sort(ord_mns.begin(), ord_mns.end(), [&](U& p1, U& p2) {
         return X_mns_Y[p1] < X_mns_Y[p2]; });
  auto grtr_x = [&](const U& p1, const U& p2) { return Xs[p1] > Xs[p2]; };
  auto grtr_y = [&](const U& p1, const U& p2) { return Ys[p1] > Ys[p2]; };
  auto less_y = [&](const U& p1, const U& p2) { return Ys[p1] < Ys[p2]; };
  span(Xs, Ys, ord_pls, X_mns_Y, grtr_x, greater<int>(), edges, adj, false);
  span(Xs, Ys, ord_pls, X_mns_Y, grtr_y, less_equal<int>(), edges, adj, true);
  span(Xs, Ys, ord_mns, X_pls_Y, less_y, less<int>(), edges, adj, false);
  span(Xs, Ys, ord_mns, X_pls_Y, grtr_x, greater_equal<int>(), edges, adj, true);
  //construct steiner-tree
  disjoint_set1<U> ds1(2*numPins - 1); 
  sort(edges.begin(), edges.end(), 
       [](const tuple<int, U, U>& t1, const tuple<int, U, U>& t2) {
         return get<0>(t1) < get<0>(t2); });
  U edge_id = 0;
  T.clear();
  T.resize(1.4*numPins, vector<bool>(1.4*numPins, false));
  vector<vector<tuple<U, U, U>>> Qs(numPins);
  vector<pair<U, U>> MBT(numPins - 1), Tedge(numPins - 1);
  LL MST_cost = 0;
  for(auto& edge : edges) {
    U &u = get<1>(edge), &v = get<2>(edge);
    U ru = ds1.root(u), rv = ds1.root(v);
    if(ru != rv) {
      MST_cost += dist(Xs[u], Ys[u], Xs[v], Ys[v]);
      T[u][v] = T[v][u] = true;
      for(auto& w : adj[u]) if(w != v) {
        if(ds1.same(ru, w)) 
          Qs[w].emplace_back(u, u, v), Qs[u].emplace_back(w, u, v);
        else if(ds1.same(rv, w)) 
          Qs[w].emplace_back(v, v, u), Qs[v].emplace_back(w, v, u);
      }
      for(auto& w : adj[v]) if(w != u) {
        if(ds1.same(ru, w)) 
          Qs[w].emplace_back(u, u, v), Qs[u].emplace_back(w, u, v);
        else if(ds1.same(rv, w)) 
          Qs[w].emplace_back(v, v, u), Qs[v].emplace_back(w, v, u);
      }
      MBT[edge_id] = {ru, rv};
      Tedge[edge_id] = {u, v};
      ds1.unite(edge_id + numPins, ds1.unite(ru, rv));
      ++edge_id;
    }
  }
  disjoint_set2<U> ds2(2*numPins - 1);
  vector<tuple<U, U, U, U, int>> ans;
  ans.reserve(1.3*numPins);
  vector<bool> colors(2*numPins - 1);
  vector<U> ancs(2*numPins - 1, -1);
  tarjan(U(edge_id + numPins - 1), numPins, ds2, Qs, MBT, ancs, colors, ans);
  #pragma omp parallel for
  for(uint i = 0; i < ans.size(); ++i)
    get<4>(ans[i]) = calc_gain(Xs, Ys, numPins, Tedge, ans[i]);
  sort(ans.begin(), ans.end(),
       [&](const tuple<U, U, U, U, int>& t1,
           const tuple<U, U, U, U, int>& t2) {
         return get<4>(t1) > get<4>(t2); });
  U Tcnt = numPins;
  for(auto &an : ans) {
    if(get<4>(an) <= 0) break;
    const U &w = get<0>(an), &u0 = get<1>(an), &v0 = get<2>(an);
    const U &x = get<3>(an), &u1 = Tedge[x - numPins].first;
    const U &v1 = Tedge[x - numPins].second;
    //assert(x >= numPins);
    //assert(u0 < numPins && v0 < numPins && u1 < numPins && v1 < numPins);
    //assert(T[u0][v0] == T[v0][u0] && T[u1][v1] == T[v1][u1]);
    if(!T[u0][v0] || !T[u1][v1]) continue;
    int nx, ny;
    tie(nx, ny) = gen_point(Xs[w], Ys[w], Xs[u0], Ys[u0], Xs[v0], Ys[v0]);
    if(nx != INT_MIN && ny != INT_MIN) {
      Xs.push_back(nx);
      Ys.push_back(ny);
      X_pls_Y.push_back(nx + ny);
      X_mns_Y.push_back(nx - ny);
      T[u0][v0] = T[v0][u0] = T[u1][v1] = T[v1][u1] = false;
      T[Tcnt][w] = T[w][Tcnt] = T[Tcnt][u0] = T[u0][Tcnt] = true;
      T[Tcnt][v0] = T[v0][Tcnt] = true;
      ++Tcnt;
    }
  }
  return MST_cost;
}
constexpr int iter = 3;
int main(int argc, char** argv) {
  int MINX, MINY, MAXX, MAXY;
  FILE *fin = fopen(argv[1] ,"r");
  fscanf(fin, "Boundary = (%d,%d), (%d,%d)\n", &MINX, &MINY, &MAXX, &MAXY);
  int numPins;
  fscanf(fin, "NumPins = %d\n", &numPins);
  vector<int> Xs(numPins), Ys(numPins), X_pls_Y(numPins), X_mns_Y(numPins);
  Xs.reserve((iter*0.3 + 1)*numPins);
  Ys.reserve((iter*0.3 + 1)*numPins);
  X_pls_Y.reserve((iter*0.3 + 1)*numPins);
  X_mns_Y.reserve((iter*0.3 + 1)*numPins);
  for(int i = 0; i < numPins; ++i) {
    fscanf(fin, "PIN %*s (%d,%d)\n", &Xs[i], &Ys[i]);
    X_pls_Y[i] = Xs[i] + Ys[i];
    X_mns_Y[i] = Xs[i] - Ys[i];
  }
  bool use_short = (6 * numPins >= 255);
  bool use_int = (6 * numPins >= 65535);
  //use_int = true;
  vector<vector<bool>> T;
  LL orig_MST_cost, MRST_cost;
  for(int i = 0; i < iter; ++i) {
    LL MST_cost;
    if(use_int)
      MST_cost = steiner_tree<int>(Xs, Ys, X_pls_Y, X_mns_Y, T);
    else if(use_short)
      MST_cost = steiner_tree<short>(Xs, Ys, X_pls_Y, X_mns_Y, T);
    else
      MST_cost = steiner_tree<char>(Xs, Ys, X_pls_Y, X_mns_Y, T);
    if(!i) orig_MST_cost = MST_cost;
    FILE *fplt = (argc == 4 ? 
                  fopen((string(argv[3]) + to_string(i)).c_str(), "w") : 0);
    MRST_cost = plot(fplt, numPins, Xs, Ys, T);
  }
  FILE *fout = fopen(argv[2], "w");
  fprintf(fout, "NumRoutedPins = %d\n", numPins);
  fprintf(fout, "Wirelength = %lld\n", MRST_cost);
  #pragma omp parallel for
  for(uint i = 0; i < T.size(); ++i)
    for(uint j = i + 1; j < T.size(); ++j) if(T[i][j]) {
      if(Xs[i] == Xs[j] && Ys[i] == Ys[j]) continue;
      else if(Xs[i] == Xs[j]) 
        fprintf(fout, "V-line (%d,%d) (%d,%d)\n", Xs[i], Ys[i], Xs[j], Ys[j]);
      else if(Ys[i] == Ys[j])
        fprintf(fout, "H-line (%d,%d) (%d,%d)\n", Xs[i], Ys[i], Xs[j], Ys[j]);
      else {
        fprintf(fout, "V-line (%d,%d) (%d,%d)\n", Xs[i], Ys[i], Xs[i], Ys[j]);
        fprintf(fout, "H-line (%d,%d) (%d,%d)\n", Xs[i], Ys[j], Xs[j], Ys[j]);
      }
  }
  cerr << "MST  cost " << orig_MST_cost << endl;
  cerr << "MRST cost " << MRST_cost << endl;
  cerr << "improvement "
       << double(orig_MST_cost - MRST_cost) / orig_MST_cost << endl;
  cerr << "numPins " << Xs.size() << endl;
}
