/**
 * @file lca.hpp
 * @brief O(n lg n) preprocessing O(log n) query Lowest Common Ancestor
 * implementation
 */

#ifndef LCA_HPP
#define LCA_HPP

#include <bit>

#include "graph.hpp"

template <class T = void, class Op = std::plus<>, bool VertexQuery = true>
class lca;

template <class Op, bool VertexQuery> class lca<void, Op, VertexQuery> {
  protected:
	size_t n, m;
	std::vector<int> depth;
	std::vector<std::vector<int>> dp;

	explicit lca(const graph<void>::dfs_traversal_result &r)
	    : lca(get<3>(r), get<2>(r)) {}

  public:
	static constexpr int no_parent = -1;

	explicit lca(const std::vector<int> &parents,
	             const std::vector<int> &depth = {})
	    : n(parents.size()), m(std::bit_width(n)), depth(depth),
	      dp(m, std::vector<int>(n, no_parent)) {
		dp[0] = parents;
		for (size_t j = 1; j < m; j++) {
			for (size_t i = 0; i < n; i++) {
				if (dp[j - 1][i] == no_parent)
					continue;
				dp[j][i] = dp[j - 1][dp[j - 1][i]];
			}
		}
	}
	explicit lca(const graph<void> &g, int root = graph<void>::all_nodes)
	    : lca(g.dfs_traversal(root)) {}

	int kth_ancestor(int u, int k) const {
		for (size_t j = 0; j < m && k > 0; j++, k >>= 1) {
			if (k & 1) {
				u = dp[j][u];
			}
			if (u == no_parent)
				return no_parent;
		}
		return k > 0 ? no_parent : u;
	}

	int operator()(int u, int v) const {
		if (depth[u] > depth[v])
			std::swap(u, v);
		while (depth[v] > depth[u])
			v = dp[std::bit_width<unsigned int>(depth[v] - depth[u]) - 1][v];
		if (u == v)
			return v;
		for (int j = std::bit_width<unsigned int>(depth[v]) - 1; j >= 0; j--)
			if (dp[j][u] != dp[j][v])
				u = dp[j][u], v = dp[j][v];
		return dp[0][u];
	}
};

template <class T, class Op, bool VertexQuery>
class lca : public lca<void, Op, VertexQuery> {
  private:
	std::vector<std::vector<T>> dat;
	Op op;

	explicit lca(const std::vector<T> &v,
	             const graph<void>::dfs_traversal_result &r, Op op = {})
	    : lca(v, get<3>(r), get<2>(r), op) {}

	using lca<void, Op, VertexQuery>::n;
	using lca<void, Op, VertexQuery>::m;
	using lca<void, Op, VertexQuery>::depth;
	using lca<void, Op, VertexQuery>::dp;

  public:
	static constexpr int no_parent = -1;

	explicit lca(const std::vector<T> &v, const std::vector<int> &parents,
	             const std::vector<int> &depth = {}, Op op = {})
	    : lca<void, Op, VertexQuery>(parents, depth), dat(m, std::vector<T>(n)),
	      op(op) {
		dat[0] = v;
		for (size_t j = 1; j < m; j++) {
			for (size_t i = 0; i < n; i++) {
				if (dp[j - 1][i] == no_parent)
					continue;
				dat[j][i] = op(dat[j - 1][i], dat[j - 1][dp[j - 1][i]]);
			}
		}
	}
	explicit lca(const std::vector<T> &v, const graph<void> &g,
	             int root = graph<void>::all_nodes, Op op = {})
	    : lca(v, g.dfs_traversal(root), op) {}

	std::pair<std::optional<T>, int> kth_ancestor(int u, int k) const {
		std::optional<T> result;
		for (int j = 0; j < m && k > 0; j++, k >>= 1) {
			if (k & 1) {
				result = result ? op(*result, dat[j][u]) : dat[j][u];
				u = dp[j][u];
			}
			if (u == no_parent)
				return {std::nullopt, no_parent};
		}
		if constexpr (VertexQuery)
			result = result ? op(*result, dat[0][u]) : dat[0][u];
		return k > 0 ? std::pair<std::optional<T>, int>{std::nullopt, no_parent}
		             : std::pair<std::optional<T>, int>{result, u};
	}

	std::pair<std::optional<T>, int> operator()(int u, int v) const {
		std::optional<T> result;
		if (depth[u] > depth[v])
			std::swap(u, v);
		while (depth[v] > depth[u]) {
			auto j = std::bit_width<unsigned int>(depth[v] - depth[u]) - 1;
			result = result ? op(*result, dat[j][v]) : dat[j][v];
			v = dp[j][v];
		}
		if (u == v) {
			if constexpr (VertexQuery)
				result = result ? op(*result, dat[0][u]) : dat[0][u];
			return {result, u};
		}
		for (int j = std::bit_width<unsigned int>(depth[v]) - 1; j >= 0; j--)
			if (dp[j][u] != dp[j][v]) {
				result = result ? op(*result, dat[j][u]) : dat[j][u];
				result = result ? op(*result, dat[j][v]) : dat[j][v];
				u = dp[j][u], v = dp[j][v];
			}
		result = result ? op(*result, dat[0][u]) : dat[0][u];
		result = result ? op(*result, dat[0][v]) : dat[0][v];
		u = dp[0][u];
		if constexpr (VertexQuery)
			result = result ? op(*result, dat[0][u]) : dat[0][u];
		return {result, u};
	}
};

#endif
