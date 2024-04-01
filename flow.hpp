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
	explicit flow_net(int n) : flow_net(n, n - 1, n - 2) {}

	void push_edge(int u, int v, T cap) {
		g.push_edge(u, v, edges.size());
		edges.push_back({cap, 0});
		g.push_edge(v, u, edges.size());
		edges.push_back({0, 0});
	}

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

#endif
