/**
 * @file classic_segment_tree.hpp
 * @brief Classic segment tree implementation with array and pointers
 */

#ifndef CLASSIC_SEGMENT_TREE_HPP
#define CLASSIC_SEGMENT_TREE_HPP

#include "functional.hpp"

template <class T, class U = T, class CombineOp = std::plus<>,
          class UpdateOp = std::plus<>>
class classic_segment_tree {
	size_t n;
	std::vector<T> tree;
	CombineOp combinator;
	UpdateOp updater;

  public:
	static constexpr size_t npos = -1;

  private:
	// 2n memory implementation
	// https://cp-algorithms.com/data_structures/segment_tree.html#memory-efficient-implementation
	constexpr size_t get_root_index() const { return 0; }
	constexpr size_t get_left_index(size_t p, size_t lo, size_t hi) const {
		return p + 1;
	}
	constexpr size_t get_right_index(size_t p, size_t lo, size_t hi) const {
		return p + ((hi - lo) / 2 + 1) * 2;
	}

	void build(const std::vector<T> &init, size_t p, size_t lo, size_t hi) {
		if (lo == hi) {
			tree[p] = init[lo];
		} else {
			size_t mi = lo + (hi - lo) / 2;
			auto ul = get_left_index(p, lo, hi),
			     ur = get_right_index(p, lo, hi);
			build(init, ul, lo, mi);
			build(init, ur, mi + 1, hi);
			tree[p] = combinator(tree[ul], tree[ur]);
		}
	}

	void modify(size_t p, const U &val, size_t u, size_t lo, size_t hi) {
		if (lo == hi) {
			tree[u] = updater(tree[u], val);
		} else {
			size_t mi = lo + (hi - lo) / 2;
			auto ul = get_left_index(u, lo, hi),
			     ur = get_right_index(u, lo, hi);
			if (p <= mi) {
				modify(p, val, ul, lo, mi);
			} else {
				modify(p, val, ur, mi + 1, hi);
			}
			tree[u] = combinator(tree[ul], tree[ur]);
		}
	}

	T query(size_t l, size_t r, size_t u, size_t lo, size_t hi) const {
		if (l == lo && r == hi) {
			return tree[u];
		}
		size_t mi = lo + (hi - lo) / 2;
		if (r <= mi) {
			return query(l, r, get_left_index(u, lo, hi), lo, mi);
		}
		if (l > mi) {
			return query(l, r, get_right_index(u, lo, hi), mi + 1, hi);
		}
		auto ul = get_left_index(u, lo, hi), ur = get_right_index(u, lo, hi);
		return combinator(query(l, mi, ul, lo, mi),
		                  query(mi + 1, r, ur, mi + 1, hi));
	}

	template <class Pred>
	size_t find(size_t l, size_t u, size_t lo, size_t hi, Pred &pred) const {
		if (!pred(tree[u])) {
			return npos;
		}
		if (lo == hi) {
			return lo;
		}
		size_t mi = lo + (hi - lo) / 2;
		if (l <= mi) {
			if (size_t l_res = find(l, get_left_index(u, lo, hi), lo, mi, pred);
			    l_res != npos) {
				return l_res;
			}
		}
		return find(l, get_right_index(u, lo, hi), mi + 1, hi, pred);
	}

  public:
	explicit classic_segment_tree(const std::vector<T> &init,
	                              CombineOp combinator = {},
	                              UpdateOp updater = {})
	    : n(init.size()), tree(n + n), combinator(combinator),
	      updater(updater) {
		build(init, get_root_index(), 0, n - 1);
	}
	explicit classic_segment_tree(int n, const T &init = {},
	                              CombineOp combinator = {},
	                              UpdateOp updater = {})
	    : classic_segment_tree(std::vector<T>(n, init), combinator, updater) {}

	size_t size() const { return n; }

	void modify(size_t p, const U &val) {
		modify(p, val, get_root_index(), 0, n - 1);
	}

	T query(size_t l, size_t r) const {
		return query(l, r, get_root_index(), 0, n - 1);
	}

	template <std::predicate<const T &> Pred>
	size_t find(size_t l, size_t r, Pred pred = {}) const {
		size_t result = find(l, get_root_index(), 0, n - 1, pred);
		return result <= r ? result : npos;
	}

	template <class R, std::predicate<const R &, const T &> BinaryPred =
	                       std::ranges::equal_to>
	size_t find(size_t l, size_t r, const R &val, BinaryPred pred = {}) const {
		return find(l, r, [&val, &pred](const T &x) { return pred(x, val); });
	}
};

template <class T, class U = T, class CombineOp = std::plus<>,
          class UpdateOp = std::plus<>, class CombineUpdateOp = UpdateOp,
          class UpdateLenOp = fn::noop>
class classic_lazy_segment_tree {
	size_t n;
	std::vector<T> tree;
	std::vector<std::optional<U>> lazy;
	CombineOp combinator;
	UpdateOp updater;
	CombineUpdateOp lazyCombinator;
	UpdateLenOp updaterLen;

  public:
	static constexpr size_t npos = -1;

  private:
	// 2n memory implementation
	// https://cp-algorithms.com/data_structures/segment_tree.html#memory-efficient-implementation
	constexpr size_t get_root_index() const { return 0; }
	constexpr size_t get_left_index(size_t p, size_t lo, size_t hi) const {
		return p + 1;
	}
	constexpr size_t get_right_index(size_t p, size_t lo, size_t hi) const {
		return p + ((hi - lo) / 2 + 1) * 2;
	}

	void build(const std::vector<T> &init, size_t p, size_t lo, size_t hi) {
		if (lo == hi) {
			tree[p] = init[lo];
		} else {
			size_t mi = lo + (hi - lo) / 2;
			auto ul = get_left_index(p, lo, hi),
			     ur = get_right_index(p, lo, hi);
			build(init, ul, lo, mi);
			build(init, ur, mi + 1, hi);
			tree[p] = combinator(tree[ul], tree[ur]);
		}
	}

	void apply(size_t p, const U &val, size_t lo, size_t hi) {
		tree[p] = updater(tree[p], updaterLen(val, hi - lo + 1));
		lazy[p] = lazy[p].has_value() ? lazyCombinator(*lazy[p], val) : val;
	}

	void push(size_t p, size_t lo, size_t hi) {
		if (lazy[p]) {
			auto mi = lo + (hi - lo) / 2;
			auto ul = get_left_index(p, lo, hi),
			     ur = get_right_index(p, lo, hi);
			apply(ul, *lazy[p], lo, mi);
			apply(ur, *lazy[p], mi + 1, hi);
			lazy[p].reset();
		}
	}

	void modify(size_t l, size_t r, const U &val, size_t u, size_t lo,
	            size_t hi) {
		if (l <= lo && hi <= r) {
			apply(u, val, lo, hi);
			return;
		}
		push(u, lo, hi);
		size_t mi = lo + (hi - lo) / 2;
		auto ul = get_left_index(u, lo, hi), ur = get_right_index(u, lo, hi);
		if (l <= mi) {
			modify(l, r, val, ul, lo, mi);
		}
		if (mi < r) {
			modify(l, r, val, ur, mi + 1, hi);
		}
		tree[u] = combinator(tree[ul], tree[ur]);
	}

	T query(size_t l, size_t r, size_t u, size_t lo, size_t hi) {
		if (l <= lo && hi <= r) {
			return tree[u];
		}
		push(u, lo, hi);
		size_t mi = lo + (hi - lo) / 2;
		auto ul = get_left_index(u, lo, hi), ur = get_right_index(u, lo, hi);
		if (r <= mi) {
			return query(l, r, ul, lo, mi);
		}
		if (mi < l) {
			return query(l, r, ur, mi + 1, hi);
		}
		return combinator(query(l, r, ul, lo, mi), query(l, r, ur, mi + 1, hi));
	}

	template <class Pred>
	size_t find(size_t l, size_t u, size_t lo, size_t hi, Pred &pred) {
		if (!pred(tree[u])) {
			return npos;
		}
		if (lo == hi) {
			return lo;
		}
		push(u, lo, hi);
		size_t mi = lo + (hi - lo) / 2;
		if (l <= mi) {
			if (size_t l_res = find(l, get_left_index(u, lo, hi), lo, mi, pred);
			    l_res != npos) {
				return l_res;
			}
		}
		return find(l, get_right_index(u, lo, hi), mi + 1, hi, pred);
	}

  public:
	explicit classic_lazy_segment_tree(const std::vector<T> &init,
	                                   CombineOp combinator = {},
	                                   UpdateOp updater = {},
	                                   CombineUpdateOp lazyCombinator = {},
	                                   UpdateLenOp updaterLen = {})
	    : n(init.size()), tree(n + n), lazy(n + n), combinator(combinator),
	      updater(updater), lazyCombinator(lazyCombinator),
	      updaterLen(updaterLen) {
		build(init, get_root_index(), 0, n - 1);
	}
	explicit classic_lazy_segment_tree(size_t n, const T &init = {},
	                                   CombineOp combinator = {},
	                                   UpdateOp updater = {},
	                                   CombineUpdateOp lazyCombinator = {},
	                                   UpdateLenOp updaterLen = {})
	    : classic_lazy_segment_tree(std::vector<T>(n, init), combinator,
	                                updater, lazyCombinator, updaterLen) {}

	void modify(int l, int r, const U &val) {
		modify(l, r, val, get_root_index(), 0, n - 1);
	}

	T query(int l, int r) { return query(l, r, get_root_index(), 0, n - 1); }

	template <std::predicate<const T &> Pred>
	size_t find(size_t l, size_t r, Pred pred = {}) {
		size_t result = find(l, get_root_index(), 0, n - 1, pred);
		return result <= r ? result : npos;
	}

	template <class R, std::predicate<const R &, const T &> BinaryPred =
	                       std::ranges::equal_to>
	size_t find(size_t l, size_t r, const R &val, BinaryPred pred = {}) {
		return find(l, r, [&val, &pred](const T &x) { return pred(x, val); });
	}
};

template <class T, class U = T, class CombineOp = std::plus<>,
          class UpdateOp = std::plus<>, class CombineUpdateOp = UpdateOp,
          class UpdateLenOp = fn::noop, std::integral SizeType = size_t>
class dynamic_segment_tree {
	SizeType n;
	std::vector<T> tree;
	std::vector<std::optional<U>> lazy;
	std::vector<size_t> left_child, right_child;
	std::function<T(SizeType, SizeType)> initializer;
	CombineOp combinator;
	UpdateOp updater;
	CombineUpdateOp lazyCombinator;
	UpdateLenOp updaterLen;
	size_t root;

	// 2n memory implementation
	// https://cp-algorithms.com/data_structures/segment_tree.html#memory-efficient-implementation
	size_t get_root_index() const { return root; }
	size_t get_left_index(size_t p, SizeType lo, SizeType hi) {
		if (!left_child[p])
			left_child[p] = create_node(lo, lo + (hi - lo) / 2);
		return left_child[p];
	}
	size_t get_right_index(size_t p, SizeType lo, SizeType hi) {
		if (!right_child[p])
			right_child[p] = create_node(lo + (hi - lo) / 2 + 1, hi);
		return right_child[p];
	}

	size_t create_node(SizeType lo, SizeType hi) {
		tree.push_back(initializer(lo, hi));
		lazy.push_back(std::nullopt);
		left_child.push_back(0);
		right_child.push_back(0);
		return tree.size() - 1;
	}

	void build(const std::vector<T> &init, size_t p, size_t lo, size_t hi) {
		if (lo == hi) {
			tree[p] = init[lo];
		} else {
			size_t mi = lo + (hi - lo) / 2;
			auto ul = get_left_index(p, lo, hi),
			     ur = get_right_index(p, lo, hi);
			build(init, ul, lo, mi);
			build(init, ur, mi + 1, hi);
			tree[p] = combinator(tree[ul], tree[ur]);
		}
	}

	void apply(size_t p, const U &val, SizeType lo, SizeType hi) {
		tree[p] = updater(tree[p], updaterLen(val, hi - lo + 1));
		lazy[p] = lazy[p].has_value() ? lazyCombinator(*lazy[p], val) : val;
	}

	void push(size_t p, SizeType lo, SizeType hi) {
		if (lazy[p]) {
			auto mi = lo + (hi - lo) / 2;
			auto ul = get_left_index(p, lo, hi),
			     ur = get_right_index(p, lo, hi);
			apply(ul, *lazy[p], lo, mi);
			apply(ur, *lazy[p], mi + 1, hi);
			lazy[p].reset();
		}
	}

	void modify(SizeType l, SizeType r, const U &val, size_t u, SizeType lo,
	            SizeType hi) {
		if (l <= lo && hi <= r) {
			apply(u, val, lo, hi);
			return;
		}
		push(u, lo, hi);
		SizeType mi = lo + (hi - lo) / 2;
		auto ul = get_left_index(u, lo, hi), ur = get_right_index(u, lo, hi);
		if (l <= mi) {
			modify(l, r, val, ul, lo, mi);
		}
		if (mi < r) {
			modify(l, r, val, ur, mi + 1, hi);
		}
		tree[u] = combinator(tree[ul], tree[ur]);
	}

	T query(SizeType l, SizeType r, size_t u, SizeType lo, SizeType hi) {
		if (l <= lo && hi <= r) {
			return tree[u];
		}
		push(u, lo, hi);
		SizeType mi = lo + (hi - lo) / 2;
		auto ul = get_left_index(u, lo, hi), ur = get_right_index(u, lo, hi);
		if (r <= mi) {
			return query(l, r, ul, lo, mi);
		}
		if (mi < l) {
			return query(l, r, ur, mi + 1, hi);
		}
		return combinator(query(l, r, ul, lo, mi), query(l, r, ur, mi + 1, hi));
	}

  public:
	explicit dynamic_segment_tree(
	    SizeType n, const std::function<T(SizeType, SizeType)> &initializer,
	    CombineOp combinator = {}, UpdateOp updater = {},
	    CombineUpdateOp lazyCombinator = {}, UpdateLenOp updaterLen = {})
	    : n(n), initializer(initializer), combinator(combinator),
	      updater(updater), lazyCombinator(lazyCombinator),
	      updaterLen(updaterLen) {
		root = create_node(0, n - 1);
	}
	explicit dynamic_segment_tree(SizeType n, const T &init = {},
	                              CombineOp combinator = {},
	                              UpdateOp updater = {},
	                              CombineUpdateOp lazyCombinator = {},
	                              UpdateLenOp updaterLen = {})
	    : dynamic_segment_tree(
	          n, [init](size_t, size_t) { return init; }, combinator, updater,
	          lazyCombinator, updaterLen) {}
	explicit dynamic_segment_tree(const std::vector<T> &init,
	                              CombineOp combinator = {},
	                              UpdateOp updater = {},
	                              CombineUpdateOp lazyCombinator = {},
	                              UpdateLenOp updaterLen = {})
	    : dynamic_segment_tree(
	          init.size(), [init](size_t u, size_t) { return init[u]; },
	          combinator, updater, lazyCombinator, updaterLen) {
		build(init, root, 0, n - 1);
	}
	void modify(SizeType l, SizeType r, const U &val) {
		modify(l, r, val, get_root_index(), 0, n - 1);
	}

	T query(SizeType l, SizeType r) {
		return query(l, r, get_root_index(), 0, n - 1);
	}
};

#endif
