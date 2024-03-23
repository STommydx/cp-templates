/**
 * @file cht.hpp
 * @brief Convex hull trick implementation
 */

#ifndef CHT_HPP
#define CHT_HPP

#include "math.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>

template <class T> class cht {
  public:
	using line = std::pair<T, T>;
	static constexpr size_t auto_assign = std::numeric_limits<size_t>::max();

  private:
	size_t n;
	std::vector<line> hull;
	std::vector<size_t> hull_line_idx;

	/**
	 * Check intersection of l1 and l2, l1 and l3. If the intersection of l1 and
	 * l3 has x coordinate not smaller than the intersection of l1 and l2, then
	 * line l2 should be skipped in the hull.
	 */
	static bool should_skip(const line &l1, const line &l2, const line &l3) {
		return fraction_cmp(l3.second - l1.second, l1.first - l3.first,
		                    l2.second - l1.second, l1.first - l2.first) >= 0;
	}

  public:
	cht() = default;
	explicit cht(const std::vector<line> &lines) : n(lines.size()) {
		std::vector<size_t> idx(lines.size());
		std::iota(idx.begin(), idx.end(), 0);
		auto indexer = [&](size_t idx) { return lines[idx].first; };
		if (!std::ranges::is_sorted(idx, std::ranges::less{}, indexer)) {
			std::ranges::sort(idx, std::ranges::less{}, indexer);
		}
		for (size_t i : idx) {
			add_line(lines[i], i);
		}
	}

	void add_line(const line &l, size_t line_idx = auto_assign) {
		while (hull.size() >= 2 &&
		       should_skip(hull[hull.size() - 2], hull[hull.size() - 1], l)) {
			hull.pop_back();
			hull_line_idx.pop_back();
		}
		hull.push_back(l);
		hull_line_idx.push_back(line_idx == auto_assign ? n++ : line_idx);
	}

	std::pair<T, size_t> query(T x) const {
		auto idx = *std::ranges::lower_bound(
		    std::views::iota(size_t{0}, hull.size() + 1), true,
		    std::ranges::less{}, [&](size_t mi) {
			    if (mi + 1 >= hull.size())
				    return true;
			    // check intersection of lines hull[mi] and hull[mi + 1]
			    // return whether x is not smaller than the x coordinate of
			    // the intersection
			    return fraction_cmp(x, T{1},
			                        hull[mi].second - hull[mi + 1].second,
			                        hull[mi + 1].first - hull[mi].first) >= 0;
		    });
		return {hull[idx].first * x + hull[idx].second, hull_line_idx[idx]};
	}

	size_t size() const { return hull.size(); }
};

template <class T, class Line = std::pair<T, T>,
          class Compare = std::ranges::less,
          class Eval = decltype([](const Line &l, T x) {
	          return l.first * x + l.second;
          })>
class lichao_tree {
  private:
	size_t n;
	T bound_lo, bound_hi;
	std::vector<std::optional<Line>> tree;
	std::vector<size_t> tree_idx;
	std::vector<size_t> left_child, right_child;
	size_t root;
	Compare comp;
	Eval eval;

  public:
	using result_type = std::invoke_result_t<Eval, Line, T>;
	static constexpr size_t auto_assign = std::numeric_limits<size_t>::max();

  private:
	size_t create_node(T lo, T hi) {
		tree.push_back(std::nullopt);
		tree_idx.push_back(0);
		left_child.push_back(0);
		right_child.push_back(0);
		return tree.size() - 1;
	}

	void push_line(Line line, size_t line_idx, size_t u, T lo, T hi) {
		if (!tree[u]) {
			tree[u] = line;
			tree_idx[u] = line_idx;
			return;
		}
		auto mi = lo + (hi - lo) / 2;
		bool dominate_left = comp(
		    std::pair<result_type, size_t>(eval(line, lo), line_idx),
		    std::pair<result_type, size_t>(eval(*tree[u], lo), tree_idx[u]));
		bool dominate_mid = comp(
		    std::pair<result_type, size_t>(eval(line, mi), line_idx),
		    std::pair<result_type, size_t>(eval(*tree[u], mi), tree_idx[u]));
		if (dominate_mid) {
			std::swap(line, *tree[u]);
			std::swap(line_idx, tree_idx[u]);
		}
		if (hi - lo == 1) {
			return;
		}
		if (dominate_left != dominate_mid) {
			// intersection at left range
			if (!left_child[u])
				left_child[u] = create_node(lo, mi);
			push_line(line, line_idx, left_child[u], lo, mi);
		} else {
			// intersection at right range
			if (!right_child[u])
				right_child[u] = create_node(mi, hi);
			push_line(line, line_idx, right_child[u], mi, hi);
		}
	}

	void modify(T l, T r, const Line &line, size_t line_idx, size_t u, T lo,
	            T hi) {
		if (l <= lo && hi <= r) {
			push_line(line, line_idx, u, lo, hi);
			return;
		}
		auto mi = lo + (hi - lo) / 2;
		if (l < mi && r > lo) {
			if (!left_child[u])
				left_child[u] = create_node(lo, mi);
			modify(l, r, line, line_idx, left_child[u], lo, mi);
		}
		if (r > mi && l < hi) {
			if (!right_child[u])
				right_child[u] = create_node(mi, hi);
			modify(l, r, line, line_idx, right_child[u], mi, hi);
		}
	}

	std::optional<std::pair<result_type, size_t>> query(T x, size_t u, T lo,
	                                                    T hi) const {
		std::optional<std::pair<result_type, size_t>> result;
		if (tree[u])
			result = {eval(*tree[u], x), tree_idx[u]};
		if (hi - lo == 1) {
			return result;
		}
		auto mi = lo + (hi - lo) / 2;
		if (x < mi) {
			if (left_child[u]) {
				auto left_result = query(x, left_child[u], lo, mi);
				if (left_result)
					result = result
					             ? std::ranges::min(*result, *left_result, comp)
					             : *left_result;
			}
		} else {
			if (right_child[u]) {
				auto right_result = query(x, right_child[u], mi, hi);
				if (right_result)
					result =
					    result ? std::ranges::min(*result, *right_result, comp)
					           : *right_result;
			}
		}
		return result;
	}

  public:
	lichao_tree(T bound_lo, T bound_hi)
	    : n(0), bound_lo(bound_lo), bound_hi(bound_hi) {
		root = create_node(bound_lo, bound_hi);
	}
	explicit lichao_tree(T bound_hi) : lichao_tree(0, bound_hi) {}
	void push_line(const Line &line, size_t line_idx = auto_assign) {
		push_line(line, line_idx == auto_assign ? n++ : line_idx, root,
		          bound_lo, bound_hi);
	}

	void modify(T l, T r, const Line &line, size_t line_idx = auto_assign) {
		modify(l, r + 1, line, line_idx == auto_assign ? n++ : line_idx, root,
		       bound_lo, bound_hi);
	}

	std::pair<result_type, size_t> query(T x) const {
		return *query(x, root, bound_lo, bound_hi);
	}
};

#endif
