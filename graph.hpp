/**
 * @file graph.hpp
 * @brief Graph data structure implementation
 */

#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <optional>
#include <vector>

template <class T = void> class graph;

template <> class graph<void> : public std::vector<std::vector<int>> {
	size_t n, m = 0;

	std::optional<vector<int>> in_degree, out_degree;

  public:
	static const int no_parent = -1;

	using edge = std::pair<int, int>;

	explicit graph(size_t n, const std::vector<edge> &edges = {})
	    : std::vector<std::vector<int>>(n), n(n) {
		for (const edge &e : edges)
			push_edge(e);
	}

	explicit graph(const std::vector<int> &parents) : graph(parents.size()) {
		for (int i = 0; i < n; i++) {
			if (parents[i] != no_parent)
				push_edge(parents[i], i);
		}
	}

	void push_edge(const edge &e) {
		(*this)[e.first].push_back(e.second);
		m++;
	}

	void push_edge(int u, int v) {
		(*this)[u].push_back(v);
		m++;
	}

	vector<int> get_in_degree() {
		if (!in_degree) {
			vector<int> ind(n);
			for (int i = 0; i < n; i++) {
				for (int v : (*this)[i])
					ind[v]++;
			}
			in_degree = std::move(ind);
		}
		return *in_degree;
	}

	vector<int> get_out_degree() {
		if (!out_degree) {
			vector<int> outd(n);
			for (int i = 0; i < n; i++) {
				outd[i] = (*this)[i].size();
			}
			out_degree = std::move(outd);
		}
		return *out_degree;
	}
};

template <bool directed = true, int base = 1>
graph<void> read_unweighted_graph(std::istream &is, int n, int m) {
	graph<void> g(n);
	for (int i = 0; i < m; i++) {
		int u, v;
		is >> u >> v;
		u -= base, v -= base;
		g.push_edge(u, v);
		if constexpr (!directed)
			g.push_edge(v, u);
	}
	return g;
}

template <class T> class graph : public graph<void> {
  public:
	using edge = std::pair<std::pair<int, int>, T>;
	std::vector<std::vector<T>> dat;

	explicit graph(size_t n, const std::vector<edge> &edges = {})
	    : graph<void>(n), dat(n) {
		for (const edge &e : edges)
			push_edge(e);
	}

	void push_edge(const edge &e) {
		dat[e.first.first].push_back(e.second);
		graph<void>::push_edge(e.first);
	}

	void push_edge(int u, int v, const T &t) {
		dat[u].push_back(t);
		graph<void>::push_edge(u, v);
	}

	template <class... Args> void emplace_edge(int u, int v, Args &&...args) {
		dat[u].emplace_back(std::forward<Args>(args)...);
		graph<void>::push_edge(u, v);
	}
};

template <class T, bool directed = true, int base = 1>
graph<T> read_graph(std::istream &is, int n, int m) {
	graph<T> g(n);
	for (int i = 0; i < m; i++) {
		int u, v;
		is >> u >> v;
		u -= base, v -= base;
		T t;
		is >> t;
		g.push_edge(u, v, t);
		if constexpr (!directed)
			g.push_edge(v, u, t);
	}
	return g;
}

#endif
