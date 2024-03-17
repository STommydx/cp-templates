/**
 * @file graph.hpp
 * @brief Graph data structure implementation
 */

#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <optional>
#include <queue>
#include <vector>

template <class T = void> class graph;

template <> class graph<void> : public std::vector<std::vector<int>> {
  protected:
	size_t n, m = 0;

  public:
	static constexpr int no_parent = -1;
	static constexpr int inf = 0x3f3f3f3f;
	static constexpr int all_nodes = -1;

	using edge = std::pair<int, int>;

	explicit graph(size_t n, const std::vector<edge> &edges = {})
	    : std::vector<std::vector<int>>(n), n(n) {
		for (const edge &e : edges)
			push_edge(e);
	}

	explicit graph(const std::vector<int> &parents) : graph(parents.size()) {
		for (size_t i = 0; i < n; i++) {
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

	vector<int> get_in_degree() const {
		vector<int> in_degree(n);
		for (size_t i = 0; i < n; i++) {
			for (int v : (*this)[i])
				in_degree[v]++;
		}
		return in_degree;
	}

	vector<int> get_out_degree() const {
		vector<int> out_degree(n);
		for (size_t i = 0; i < n; i++) {
			out_degree[i] = (*this)[i].size();
		}
		return out_degree;
	}

	graph<void> reversed() const {
		graph<void> g(n);
		for (size_t u = 0; u < n; u++) {
			for (int v : (*this)[u]) {
				g.push_edge(v, u);
			}
		}
		return g;
	}

	std::vector<std::vector<bool>> adjacency_matrix() const {
		std::vector<std::vector<bool>> adj(n, std::vector<bool>(n, false));
		for (size_t u = 0; u < n; u++) {
			for (int v : (*this)[u]) {
				adj[u][v] = true;
			}
		}
		return adj;
	}

	std::vector<edge> edge_list() const {
		std::vector<edge> res;
		for (size_t u = 0; u < n; u++) {
			for (int v : (*this)[u]) {
				res.emplace_back(u, v);
			}
		}
		return res;
	}

	using connected_components_result =
	    std::pair<std::vector<int>, std::vector<vector<int>>>;
	connected_components_result get_connected_components() const {
		const int uncolored = -1;
		int total_colors = 0;
		vector<int> color(n, uncolored);
		auto dfs = [&](auto &self, int u) -> void {
			for (int v : (*this)[u]) {
				if (color[v] == uncolored) {
					color[v] = color[u];
					self(self, v);
				}
			}
		};
		for (size_t u = 0; u < n; u++) {
			if (color[u] == uncolored) {
				color[u] = total_colors++;
				dfs(dfs, u);
			}
		}
		std::vector<std::vector<int>> components(total_colors);
		for (size_t u = 0; u < n; u++) {
			components[color[u]].push_back(u);
		}
		return {std::move(color), std::move(components)};
	}

	using dfs_traversal_result = std::tuple<std::vector<int>, std::vector<int>,
	                                        std::vector<int>, std::vector<int>>;
	dfs_traversal_result dfs_traversal(int start_node = all_nodes) const {
		vector<int> pre_order, post_order;
		vector<int> depth(n, inf), parent(n, no_parent);
		auto dfs = [&](auto &self, int u) -> void {
			pre_order.push_back(u);
			for (int v : (*this)[u]) {
				if (depth[v] == inf) {
					depth[v] = depth[u] + 1;
					parent[v] = u;
					self(self, v);
				}
			}
			post_order.push_back(u);
		};
		if (start_node != all_nodes) {
			depth[start_node] = 0;
			dfs(dfs, start_node);
		} else {
			for (size_t i = 0; i < n; i++) {
				if (depth[i] == inf) {
					depth[i] = 0;
					dfs(dfs, i);
				}
			}
		}
		return {std::move(pre_order), std::move(post_order), std::move(depth),
		        std::move(parent)};
	}

	using bfs_traversal_result =
	    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>;
	bfs_traversal_result bfs_traversal(int start_node = all_nodes) const {
		vector<int> order;
		vector<int> distance(n, inf), parent(n, no_parent);
		auto bfs = [&](int s) -> void {
			std::queue<int> q;
			q.push(s);
			distance[s] = 0;
			while (!q.empty()) {
				int u = q.front();
				q.pop();
				order.push_back(u);
				for (int v : (*this)[u]) {
					if (distance[v] == inf) {
						distance[v] = distance[u] + 1;
						parent[v] = u;
						q.push(v);
					}
				}
			}
		};
		if (start_node != all_nodes) {
			bfs(start_node);
		} else {
			for (size_t i = 0; i < n; i++) {
				if (distance[i] == inf) {
					bfs(i);
				}
			}
		}
		return {std::move(order), std::move(distance), std::move(parent)};
	}

	std::optional<std::vector<int>> topological_sort() const {
		vector<int> in_degree = get_in_degree();
		vector<int> result;
		vector<int> q;
		for (size_t i = 0; i < n; i++) {
			if (in_degree[i] == 0) {
				q.push_back(i);
			}
		}
		while (!q.empty()) {
			int u = q.back();
			q.pop_back();
			result.push_back(u);
			for (int v : (*this)[u]) {
				in_degree[v]--;
				if (in_degree[v] == 0) {
					q.push_back(v);
				}
			}
		}
		if (result.size() != n) {
			return std::nullopt;
		}
		return result;
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

	std::vector<std::vector<T>> adjacency_matrix() const {
		std::vector<std::vector<T>> adj(n, std::vector<T>(n));
		for (size_t u = 0; u < n; u++) {
			for (size_t i = 0; i < dat[u].size(); i++) {
				adj[u][(*this)[u][i]] = dat[u][i];
			}
		}
		return adj;
	}

	std::vector<edge> edge_list() const {
		std::vector<edge> res;
		for (size_t u = 0; u < n; u++) {
			for (size_t i = 0; i < dat[u].size(); i++) {
				res.emplace_back(std::pair<int, int>(u, (*this)[u][i]),
				                 dat[u][i]);
			}
		}
		return res;
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
