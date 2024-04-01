/**
 * @file flow.hpp
 * @brief Network flow related algorithms
 */

#ifndef FLOW_HPP
#define FLOW_HPP

#include <vector>

#include "graph.hpp"
#include "limits.hpp"

template <class T> class flow_net {
  private:
	struct edge {
		T cap, flow;
	};
	int n;
	graph<int> g;
	std::vector<edge> edges;
	int source, sink;

  public:
	static constexpr int inf = cp_limits<int>::infinity();

	flow_net(int n, int source, int sink)
	    : n(n), g(n), source(source), sink(sink) {}
	explicit flow_net(int n) : flow_net(n, n - 2, n - 1) {}

	void push_edge(int u, int v, T cap) {
		g.push_edge(u, v, edges.size());
		edges.push_back({cap, 0});
		g.push_edge(v, u, edges.size());
		edges.push_back({0, 0});
	}

	void set_k_flow(int k) {
		int new_source = n++;
		g.resize(n);
		push_edge(new_source, source, k);
		source = new_source;
	}

	int get_source() const { return source; }
	int get_sink() const { return sink; }

	T dinic_max_flow() {
		T max_flow = 0;
		std::vector<int> distance, visited(n);
		auto bfs = [&]() {
			distance.assign(n, inf);
			std::queue<int> q;
			q.push(source);
			distance[source] = 0;
			while (!q.empty()) {
				int u = q.front();
				q.pop();
				if (u == sink)
					break;
				for (int i = 0; i < std::ssize(g[u]); i++) {
					int ei = g.dat[u][i];
					edge &e = edges[ei];
					int v = g[u][i];
					if (distance[v] == inf && e.flow < e.cap) {
						distance[v] = distance[u] + 1;
						q.push(v);
					}
				}
			}
			return distance[sink] != inf;
		};
		auto dfs = [&](auto &self, int u, T flow) -> T {
			if (u == sink || flow == 0)
				return flow;
			T current_flow = 0;
			for (; visited[u] < std::ssize(g[u]); visited[u]++) {
				int ei = g.dat[u][visited[u]];
				edge &e = edges[ei];
				int v = g[u][visited[u]];
				if (distance[v] != distance[u] + 1)
					continue;
				T can_push = self(
				    self, v, std::min(flow - current_flow, e.cap - e.flow));
				if (can_push > 0) {
					e.flow += can_push;
					edges[ei ^ 1].flow -= can_push;
					current_flow += can_push;
					if (current_flow == flow)
						return current_flow;
				}
			}
			return current_flow;
		};
		while (bfs()) {
			visited.assign(n, 0);
			max_flow += dfs(dfs, source, cp_limits<T>::infinity());
		}
		return max_flow;
	}
};

template <class T> class cost_flow_net {
  private:
	struct edge {
		T cap, flow, cost;
	};
	int n;
	graph<int> g;
	std::vector<edge> edges;
	int source, sink;

  public:
	static constexpr int inf = cp_limits<int>::infinity();

	cost_flow_net(int n, int source, int sink)
	    : n(n), g(n), source(source), sink(sink) {}
	explicit cost_flow_net(int n) : cost_flow_net(n, n - 2, n - 1) {}

	void push_edge(int u, int v, T cap, T cost) {
		g.push_edge(u, v, edges.size());
		edges.push_back({cap, 0, cost});
		g.push_edge(v, u, edges.size());
		edges.push_back({0, 0, -cost});
	}

	void set_k_flow(int k) {
		int new_source = n++;
		g.resize(n);
		push_edge(new_source, source, k, 0);
		source = new_source;
	}

	int get_source() const { return source; }
	int get_sink() const { return sink; }

	std::pair<T, T> dinic_mcmf() {
		T max_flow = 0, total_cost = 0;
		std::vector<T> distance;
		std::vector<int> visited(n), current_index(n);
		auto spfa = [&]() {
			distance.assign(n, cp_limits<T>::infinity());
			std::vector<int> in_queue(n);
			std::queue<int> q;
			q.push(source);
			distance[source] = 0;
			in_queue[source] = 1;
			while (!q.empty()) {
				int u = q.front();
				q.pop();
				in_queue[u] = 0;
				for (int i = 0; i < std::ssize(g[u]); i++) {
					int ei = g.dat[u][i];
					edge &e = edges[ei];
					int v = g[u][i];
					if (e.flow >= e.cap)
						continue;
					if (T new_dist = distance[u] + e.cost;
					    new_dist < distance[v]) {
						distance[v] = new_dist;
						if (!in_queue[v])
							q.push(v), in_queue[v] = 1;
					}
				}
			}
			return distance[sink] != cp_limits<T>::infinity();
		};
		auto dfs = [&](auto &self, int u, T flow) -> T {
			if (u == sink || flow == 0)
				return flow;
			T current_flow = 0;
			visited[u] = true;
			for (int &i = current_index[u]; i < std::ssize(g[u]); i++) {
				int ei = g.dat[u][i];
				edge &e = edges[ei];
				int v = g[u][i];
				if (distance[v] != distance[u] + e.cost)
					continue;
				if (visited[v])
					continue;
				T can_push = self(
				    self, v, std::min(flow - current_flow, e.cap - e.flow));
				if (can_push > 0) {
					e.flow += can_push;
					edges[ei ^ 1].flow -= can_push;
					current_flow += can_push;
					total_cost += can_push * e.cost;
					if (current_flow == flow)
						break;
				}
			}
			visited[u] = false;
			return current_flow;
		};
		while (spfa()) {
			current_index.assign(n, 0);
			T current_flow;
			while ((current_flow = dfs(dfs, source, cp_limits<T>::infinity())) >
			       0)
				max_flow += current_flow;
		}
		return {total_cost, max_flow};
	}
};

#endif
