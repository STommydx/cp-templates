/**
 * @file graph.hpp
 * @brief Graph data structure implementation
 */

#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <optional>
#include <queue>
#include <tuple>
#include <vector>

#include "io.hpp"
#include "limits.hpp"

template <class T = void> class graph;

template <> class graph<void> : public std::vector<std::vector<int>> {
  protected:
	size_t n, m = 0;

  public:
	static constexpr int no_parent = -1;
	static constexpr int inf = cp_limits<int>::infinity();
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

	void push_undirected_edge(const edge &e) {
		push_edge(e);
		(*this)[e.second].push_back(e.first);
		m++;
	}

	void push_edge(int u, int v) {
		(*this)[u].push_back(v);
		m++;
	}

	void push_undirected_edge(int u, int v) {
		push_edge(u, v);
		push_edge(v, u);
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
		std::vector<int> color(n, uncolored);
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
		std::vector<int> pre_order, post_order;
		std::vector<int> depth(n, inf), parent(n, no_parent);
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
		std::vector<int> order;
		std::vector<int> distance(n, inf), parent(n, no_parent);
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
		std::vector<int> in_degree = get_in_degree();
		std::vector<int> result;
		std::vector<int> q;
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

	using subtree_size_result = std::pair<std::vector<int>, std::vector<int>>;
	subtree_size_result subtree_size(int root) const {
		std::vector<int> sz(n), heavy(n, no_parent);
		auto dfs = [&](auto &self, int u, int p) -> void {
			sz[u] = 1;
			for (int v : (*this)[u]) {
				if (v - p) {
					self(self, v, u);
					sz[u] += sz[v];
					if (heavy[u] == no_parent || sz[v] > sz[heavy[u]]) {
						heavy[u] = v;
					}
				}
			}
		};
		dfs(dfs, root, -1);
		return {std::move(sz), std::move(heavy)};
	}

	using flatten_result =
	    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>;
	flatten_result flatten(int start_node = all_nodes) const {
		std::vector<int> order, in_time(n, no_parent), out_time(n, no_parent);
		int timer = 0;
		auto dfs = [&](auto &self, int u) -> void {
			in_time[u] = timer++;
			order.push_back(u);
			for (int v : (*this)[u]) {
				if (in_time[v] == no_parent) {
					self(self, v);
				}
			}
			out_time[u] = timer;
		};
		if (start_node != all_nodes) {
			dfs(dfs, start_node);
		} else {
			for (size_t i = 0; i < n; i++) {
				if (in_time[i] == no_parent) {
					dfs(dfs, i);
				}
			}
		}
		return {std::move(order), std::move(in_time), std::move(out_time)};
	}
};

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

	void push_undirected_edge(const edge &e) {
		dat[e.first.first].push_back(e.second);
		dat[e.first.second].push_back(e.second);
		graph<void>::push_undirected_edge(e.first);
	}

	void push_edge(int u, int v, const T &t) {
		dat[u].push_back(t);
		graph<void>::push_edge(u, v);
	}

	void push_undirected_edge(int u, int v, const T &t) {
		push_edge(u, v, t);
		push_edge(v, u, t);
	}

	template <class... Args> void emplace_edge(int u, int v, Args &&...args) {
		dat[u].emplace_back(std::forward<Args>(args)...);
		graph<void>::push_edge(u, v);
	}

	void resize(size_t count) {
		graph<void>::resize(count);
		dat.resize(count);
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

	using dijkstra_result = std::pair<std::vector<T>, std::vector<int>>;
	dijkstra_result dijkstra(int start_node) const {
		std::vector<T> distance(n, cp_limits<T>::infinity());
		std::vector<int> parent(n, no_parent);
		std::vector<int> visited(n);
		std::priority_queue<std::pair<T, int>, std::vector<std::pair<T, int>>,
		                    std::greater<std::pair<T, int>>>
		    q;
		q.emplace(distance[start_node] = 0, start_node);
		while (!q.empty()) {
			int u = q.top().second;
			q.pop();
			if (visited[u]) {
				continue;
			}
			visited[u] = true;
			for (size_t j = 0; j < dat[u].size(); j++) {
				int v = (*this)[u][j];
				T edge_weight = dat[u][j];
				if (T new_distance = distance[u] + edge_weight;
				    new_distance < distance[v]) {
					q.emplace(distance[v] = new_distance, v);
					parent[v] = u;
				}
			}
		}
		return {std::move(distance), std::move(parent)};
	}

	using zero_one_bfs_result = std::pair<std::vector<T>, std::vector<int>>;
	zero_one_bfs_result zero_one_bfs(int start_node) const {
		std::vector<T> distance(n, cp_limits<T>::infinity());
		std::vector<int> parent(n, no_parent);
		std::vector<int> visited(n);
		std::deque<int> q;
		distance[start_node] = 0;
		q.push_front(start_node);
		while (!q.empty()) {
			int u = q.front();
			q.pop_front();
			if (visited[u]) {
				continue;
			}
			visited[u] = true;
			for (size_t j = 0; j < dat[u].size(); j++) {
				int v = (*this)[u][j];
				T edge_weight = dat[u][j];
				if (T new_distance = distance[u] + edge_weight;
				    new_distance < distance[v]) {
					distance[v] = new_distance;
					parent[v] = u;
					if (edge_weight == 0) {
						q.push_front(v);
					} else {
						q.push_back(v);
					}
				}
			}
		}
		return {std::move(distance), std::move(parent)};
	}

	using spfa_result = std::tuple<std::vector<T>, std::vector<int>, bool>;
	spfa_result spfa(int start_node) const {
		std::vector<T> distance(n, cp_limits<T>::infinity());
		std::vector<int> edge_distance(n);
		std::vector<int> parent(n, no_parent);
		std::vector<int> in_queue(n);
		std::queue<int> q;
		q.push(start_node);
		distance[start_node] = 0;
		edge_distance[start_node] = 0;
		in_queue[start_node] = true;
		while (!q.empty()) {
			int u = q.front();
			q.pop();
			in_queue[u] = false;
			for (size_t j = 0; j < dat[u].size(); j++) {
				int v = (*this)[u][j];
				T edge_weight = dat[u][j];
				if (T new_distance = distance[u] + edge_weight;
				    new_distance < distance[v]) {
					distance[v] = new_distance;
					parent[v] = u;
					// negative cycle detection
					edge_distance[v] = edge_distance[u] + 1;
					if (edge_distance[v] > static_cast<int>(n)) {
						return {std::move(distance), std::move(parent), true};
					}
					// push only if not in queue
					if (!in_queue[v]) {
						q.push(v);
						in_queue[v] = true;
					}
				}
			}
		}
		return {std::move(distance), std::move(parent), false};
	}
};

/**
 * c++ style I/O for graph similar to std::get_time, std::chrono::parse
 */
template <class T, bool directed = true, int base = 1> struct _get_graph {
	graph<T> &g;
	int m;
};

template <class T, bool directed, int base>
std::istream &operator>>(std::istream &is,
                         _get_graph<T, directed, base> get_g) {
	for (int i = 0; i < get_g.m; i++) {
		typename graph<T>::edge e;
		is >> get_indices<std::pair<int, int>, base>(e.first) >> e.second;
		if constexpr (directed)
			get_g.g.push_edge(e);
		else
			get_g.g.push_undirected_edge(e);
	}
	return is;
}

template <bool directed, int base>
std::istream &operator>>(std::istream &is,
                         _get_graph<void, directed, base> get_g) {
	for (int i = 0; i < get_g.m; i++) {
		graph<void>::edge e;
		is >> get_indices<graph<void>::edge, base>(e);
		if constexpr (directed)
			get_g.g.push_edge(e);
		else
			get_g.g.push_undirected_edge(e);
	}
	return is;
}

template <class T, bool directed = true, int base = 1>
_get_graph<T, directed, base> get_graph(graph<T> &g, int m) {
	return _get_graph<T, directed, base>{g, m};
}

template <class T, int base = 1>
_get_graph<T, true, base> get_directed_graph(graph<T> &g, int m) {
	return _get_graph<T, true, base>{g, m};
}

template <class T, int base = 1>
_get_graph<T, false, base> get_undirected_graph(graph<T> &g, int m) {
	return _get_graph<T, false, base>{g, m};
}

template <class T, bool directed = true, int base = 1>
_get_graph<T, directed, base> get_tree(graph<T> &g) {
	return _get_graph<T, directed, base>{g,
	                                     static_cast<int>(std::ssize(g) - 1)};
}

template <class T, int base = 1>
_get_graph<T, true, base> get_directed_tree(graph<T> &g) {
	return _get_graph<T, true, base>{g, static_cast<int>(std::ssize(g) - 1)};
}

template <class T, int base = 1>
_get_graph<T, false, base> get_undirected_tree(graph<T> &g) {
	return _get_graph<T, false, base>{g, static_cast<int>(std::ssize(g) - 1)};
}

#endif
