#include <algorithm>
#include <cstdio>
#include <numeric>
#include <set>
#include <tuple>
#include <vector>
#include <iostream>

using namespace std;
typedef long long LL;

inline void plot_rect(FILE *f, int x0, int y0, int x1, int y1) {
  if(x0 == x1 || y0 == y1)
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", x0, y0, x1, y1); 
  else {
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", x0, y0, x0, y1);
    fprintf(f, "set arrow from %d,%d to %d,%d nohead\n", x0, y1, x1, y1); 
  }
}
inline void out_rect_line(FILE *f, int x0, int y0, int x1, int y1) {
  if(x0 == x1 && y0 == y1) return;
  else if(x0 == x1)
    fprintf(f, "V-line (%d,%d) (%d,%d)\n", x0, y0, x1, y1);
  else if(y0 == y1)
    fprintf(f, "H-line (%d,%d) (%d,%d)\n", x0, y0, x1, y1);
  else {
    fprintf(f, "V-line (%d,%d) (%d,%d)\n", x0, y0, x0, y1);
    fprintf(f, "H-line (%d,%d) (%d,%d)\n", x0, y1, x1, y1);
  }
}
inline LL dist(const int& x0, const int& y0, const int& x1, const int& y1) {
  return abs(x0 - x1) + abs(y0 - y1);
}
template<typename U> inline void
plot(FILE *fplt, FILE *fout, const U& nPins, const LL& MRST_cost, 
     const vector<int>& Xs, const vector<int>& Ys, 
     const vector<pair<U, U>>& Tedge, const vector<bool>& T,
     const vector<pair<U, U>>& new_edge) {
  fprintf(fout, "NumRoutedPins = %d\n", nPins);
  fprintf(fout, "Wirelength = %lld\n", MRST_cost);
  if(fplt) fprintf(fplt, "set size ratio -1\nset nokey\n");
  if(fplt) fprintf(fplt, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7\n");
  #pragma omp parallel for
  for(uint i = 0; i < Tedge.size(); ++i) if(T[i]) {
    const int &u = Tedge[i].first, &v = Tedge[i].second;
    if(fplt) plot_rect(fplt, Xs[u], Ys[u], Xs[v], Ys[v]);
    out_rect_line(fout, Xs[u], Ys[u], Xs[v], Ys[v]);
  }
  #pragma omp parallel for
  for(uint i = 0; i < new_edge.size(); ++i) {
    const pair<U, U>& e = new_edge[i];
    if(fplt)
      plot_rect(fplt, Xs[e.first], Ys[e.first], Xs[e.second], Ys[e.second]);
    out_rect_line(fout, Xs[e.first], Ys[e.first], Xs[e.second], Ys[e.second]);
  }
  if(fplt) {
    fprintf(fplt, "set style line 1 lc rgb 'blue' pt 5\n");
    fprintf(fplt, "set style line 2 lc rgb 'red' pt 7\n");
    fprintf(fplt, "plot '-' w p ls 1, '-' w p ls 2\n");
    for(int i = 0; i < nPins; ++i) fprintf(fplt, "%d %d\n", Xs[i], Ys[i]);
    fprintf(fplt, "e\n");
    for(uint i = nPins; i < Xs.size(); ++i)
      fprintf(fplt, "%d %d\n", Xs[i], Ys[i]);
    fprintf(fplt, "e\npause -1\n");
  }
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
  vector<U> par;
};
template<typename U>
inline void tarjan(const U& u, const U& nPins, disjoint_set2<U>& ds2, 
            vector<vector<tuple<U, U>>>& Qs, const vector<pair<U, U>>& MBT, 
            const vector<pair<U, U>>& Tedge, vector<U>& ancs, 
            vector<bool>& colors, vector<tuple<U, U, U, int>>& ans) {
  ancs[u] = u;
  if(u >= nPins) {
    tarjan(MBT[u - nPins].first,  nPins, ds2, Qs, MBT, Tedge, ancs, colors, ans);
    ds2.unite(u, MBT[u - nPins].first);
    ancs[ds2.root(u)] = u;
    tarjan(MBT[u - nPins].second, nPins, ds2, Qs, MBT, Tedge, ancs, colors, ans);
    ds2.unite(u, MBT[u - nPins].second);
    ancs[ds2.root(u)] = u;
  }
  colors[u] = true;
  if(u < nPins) for(auto &Q : Qs[u]) if(colors[get<0>(Q)])
    if(u == Tedge[get<1>(Q)].first || u == Tedge[get<1>(Q)].second)
      ans.emplace_back(get<0>(Q), get<1>(Q), ancs[ds2.root(get<0>(Q))]-nPins, 0);
    else
      ans.emplace_back(u, get<1>(Q), ancs[ds2.root(get<0>(Q))]-nPins, 0);
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
                     const U& nPins, const vector<pair<U, U>>& Tedge,
                     const tuple<U, U, U, int>& t) {
  const U &w = get<0>(t), &u0 = Tedge[get<1>(t)].first;
  const U &v0 = Tedge[get<1>(t)].second;
  const U &u1 = Tedge[get<2>(t)].first, &v1 = Tedge[get<2>(t)].second;
  return dist(Xs[u1], Ys[u1], Xs[v1], Ys[v1]) -
         calc_cost(Xs[w], Ys[w], Xs[u0], Ys[u0], Xs[v0], Ys[v0]);
}
inline pair<int, int> gen_point(int x0, int y0, int x1, int y1, int x2, int y2) {
  int mxx = max(x1, x2), mnx = min(x1, x2);
  int mxy = max(y1, y2), mny = min(y1, y2);
  int rtnx = x0, rtny = y0;
  if(x0 < mnx) rtnx = mnx;
  else if(x0 > mxx) rtnx = mxx;
  if(y0 < mny) rtny = mny;
  else if(y0 > mxy) rtny = mxy;
  return {rtnx, rtny};
}
template<typename U> inline pair<LL, LL> 
steiner_tree(vector<int>& Xs, vector<int>& Ys, vector<int>& X_pls_Y,
             vector<int>& X_mns_Y, vector<pair<U, U>>& Tedge, vector<bool>& T,
             vector<pair<U, U>>& new_edge, bool add_new_edge) {
  U nPins = (U)Xs.size();
  //construct spanning graph
  vector<tuple<int, U, U>> edges;
  vector<vector<U>> adj(nPins);
  edges.reserve(3 * nPins);
  vector<U> ord_pls(nPins), ord_mns(nPins);
  iota(ord_pls.begin(), ord_pls.end(), 0);
  iota(ord_mns.begin(), ord_mns.end(), 0);
  sort(ord_pls.begin(), ord_pls.end(), [&](const U& p1, const U& p2) {
         return X_pls_Y[p1] < X_pls_Y[p2]; });
  sort(ord_mns.begin(), ord_mns.end(), [&](const U& p1, const U& p2) {
         return X_mns_Y[p1] < X_mns_Y[p2]; });
  auto grtr_x = [&](const U& p1, const U& p2) { return Xs[p1] > Xs[p2]; };
  auto grtr_y = [&](const U& p1, const U& p2) { return Ys[p1] > Ys[p2]; };
  auto less_y = [&](const U& p1, const U& p2) { return Ys[p1] < Ys[p2]; };
  span(Xs, Ys, ord_pls, X_mns_Y, grtr_x, greater<int>(), edges, adj, 0);
  span(Xs, Ys, ord_pls, X_mns_Y, grtr_y, less_equal<int>(), edges, adj, 1);
  span(Xs, Ys, ord_mns, X_pls_Y, less_y, less<int>(), edges, adj, 0);
  span(Xs, Ys, ord_mns, X_pls_Y, grtr_x, greater_equal<int>(), edges, adj, 1);
  //construct steiner-tree
  disjoint_set1<U> ds1(2*nPins - 1); 
  sort(edges.begin(), edges.end(), 
       [&](const tuple<int, U, U>& t0, const tuple<int, U, U>& t1) {
       return get<0>(t0) > get<0>(t1); });
  reverse(edges.begin(), edges.end());
  U edge_id = 0;
  Tedge.resize(nPins - 1);
  vector<vector<tuple<U, U>>> Qs(nPins);
  vector<pair<U, U>> MBT(nPins - 1);
  LL MST_cost = 0;
  for(auto& edge : edges) {
    U &u = get<1>(edge), &v = get<2>(edge);
    U ru = ds1.root(u), rv = ds1.root(v);
    if(ru != rv) {
      MST_cost += dist(Xs[u], Ys[u], Xs[v], Ys[v]);
      for(auto& w : adj[u]) if(w != v) {
        if(ds1.same(ru, w)) 
          Qs[w].emplace_back(u, edge_id), Qs[u].emplace_back(w, edge_id);
        else if(ds1.same(rv, w)) 
          Qs[w].emplace_back(v, edge_id), Qs[v].emplace_back(w, edge_id);
        else {
          Qs[w].emplace_back(u, edge_id), Qs[u].emplace_back(w, edge_id);
          Qs[w].emplace_back(v, edge_id), Qs[v].emplace_back(w, edge_id);
        }
      }
      for(auto& w : adj[v]) if(w != u) {
        if(ds1.same(ru, w)) 
          Qs[w].emplace_back(u, edge_id), Qs[u].emplace_back(w, edge_id);
        else if(ds1.same(rv, w)) 
          Qs[w].emplace_back(v, edge_id), Qs[v].emplace_back(w, edge_id);
        else {
          Qs[w].emplace_back(u, edge_id), Qs[u].emplace_back(w, edge_id);
          Qs[w].emplace_back(v, edge_id), Qs[v].emplace_back(w, edge_id);
        }
      }
      MBT[edge_id] = {ru, rv};
      Tedge[edge_id] = {u, v};
      ds1.unite(edge_id + nPins, ds1.unite(ru, rv));
      ++edge_id;
    }
  }
  disjoint_set2<U> ds2(2*nPins - 1);
  vector<tuple<U, U, U, int>> ans;
  ans.reserve(1.3*nPins);
  vector<bool> colors(2*nPins - 1);
  vector<U> ancs(2*nPins - 1, -1);
  tarjan(U(edge_id + nPins - 1), nPins, ds2, Qs, MBT, Tedge, ancs, colors, ans);
  #pragma omp parallel for
  for(uint i = 0; i < ans.size(); ++i)
    get<3>(ans[i]) = calc_gain(Xs, Ys, nPins, Tedge, ans[i]);
  sort(ans.begin(), ans.end(),
       [&](const tuple<U, U, U, int>& t1, const tuple<U, U, U, int>& t2) {
         return get<3>(t1) > get<3>(t2); });
  U Tcnt = nPins;
  LL MRST_cost = MST_cost;
  T.clear();
  T.resize(nPins - 1, true);
  for(auto &an : ans) {
    if(get<3>(an) <= 0) break;
    if(!T[get<1>(an)] || !T[get<2>(an)]) continue;
    const U &w = get<0>(an);
    const U &u0 = Tedge[get<1>(an)].first, &v0 = Tedge[get<1>(an)].second;
    const U &u1 = Tedge[get<2>(an)].first;
    const U &v1 = Tedge[get<2>(an)].second;
    MRST_cost -= get<3>(an);
    int nx, ny;
    tie(nx, ny) = gen_point(Xs[w], Ys[w], Xs[u0], Ys[u0], Xs[v0], Ys[v0]);
    Xs.push_back(nx);
    Ys.push_back(ny);
    X_pls_Y.push_back(nx + ny);
    X_mns_Y.push_back(nx - ny);
    T[get<1>(an)] = T[get<2>(an)] = false;
    if(add_new_edge) {
      new_edge.emplace_back(Tcnt, w);
      new_edge.emplace_back(Tcnt, u0);
      new_edge.emplace_back(Tcnt, v0);
    }
    ++Tcnt;
  }
  return {MST_cost, MRST_cost};
}
int main(int argc, char** argv) {
  int MINX, MINY, MAXX, MAXY, iter = 2, nPins;
  FILE *fin = fopen(argv[1] ,"r"), *fout = fopen(argv[2], "w");
  fscanf(fin, "Boundary = (%d,%d), (%d,%d)\n", &MINX, &MINY, &MAXX, &MAXY);
  fscanf(fin, "NumPins = %d\n", &nPins);
  if(nPins >= 500) ++iter;
  if(nPins >= 2000) ++iter;
  if(nPins >= 10000) ++iter;
  if(nPins >= 50000) ++iter;
  if(nPins >= 200000) ++iter;
  vector<int> Xs(nPins), Ys(nPins), X_pls_Y(nPins), X_mns_Y(nPins);
  Xs.reserve((iter*0.3 + 1)*nPins);
  Ys.reserve((iter*0.3 + 1)*nPins);
  X_pls_Y.reserve((iter*0.3 + 1)*nPins);
  X_mns_Y.reserve((iter*0.3 + 1)*nPins);
  for(int i = 0; i < nPins; ++i) {
    fscanf(fin, "PIN %*s (%d,%d)\n", &Xs[i], &Ys[i]);
    X_pls_Y[i] = Xs[i] + Ys[i];
    X_mns_Y[i] = Xs[i] - Ys[i];
  }
  vector<bool> T;
  LL orig_MST_cost, MRST_cost;
  for(int i = 0; i < iter; ++i) {
    FILE *fplt = (argc == 4 ? 
                  fopen((string(argv[3]) + to_string(i)).c_str(), "w") : 0);
    LL MST_cost;
    bool use_short = (3 * Xs.size() >= 255);
    bool use_int = (3 * Xs.size() >= 65535);
    bool addedge = (i+1 == iter);
    if(use_int) {
      vector<pair<int, int>> new_edge, Tedge;
      new_edge.reserve(nPins);
      tie(MST_cost, MRST_cost) = 
        steiner_tree<int>(Xs, Ys, X_pls_Y, X_mns_Y, 
                          Tedge, T, new_edge, addedge);
      if(addedge) 
        plot(fplt, fout, (int)nPins, MRST_cost, Xs, Ys, Tedge, T, new_edge);
    } else if(use_short) {
      vector<pair<short, short>> new_edge, Tedge;
      new_edge.reserve(nPins);
      tie(MST_cost, MRST_cost) =
        steiner_tree<short>(Xs, Ys, X_pls_Y, X_mns_Y, 
                            Tedge, T, new_edge, addedge);
      if(addedge) 
        plot(fplt, fout, (short)nPins, MRST_cost, Xs, Ys, Tedge, T, new_edge);
    } else {
      vector<pair<char, char>> new_edge, Tedge;
      new_edge.reserve(nPins);
      tie(MST_cost, MRST_cost) =
        steiner_tree<char>(Xs, Ys, X_pls_Y, X_mns_Y,
                           Tedge, T, new_edge, addedge);
      if(addedge) 
        plot(fplt, fout, (char)nPins, MRST_cost, Xs, Ys, Tedge, T, new_edge);
    }
    //fprintf(stderr, "tmp_MRST_cost = %lld\n", MRST_cost);
    if(!i) orig_MST_cost = MST_cost;
    if(fplt) fclose(fplt);
  }
  fclose(fout);
  fprintf(stderr, "MST_cost = %lld\n", orig_MST_cost);
  fprintf(stderr, "MRST_cost = %lld\n", MRST_cost);
  fprintf(stderr, "improvement = %.6f\n", 
          double(orig_MST_cost - MRST_cost) / orig_MST_cost);
}
