/**
 * @file segment_tree.hpp
 * @brief Segment tree data structure implementation
 *
 * This is an generic implementation of segment tree data structure based on
 * https://codeforces.com/blog/entry/18051.
 */

#ifndef SEGMENT_TREE_HPP
#define SEGMENT_TREE_HPP

#include <algorithm>
#include <bit>
#include <functional>
#include <optional>
#include <vector>

#include "functional.hpp"

template <class T, class U = T, class CombineOp = std::plus<>,
          class UpdateOp = std::plus<>>
class segment_tree {
	size_t n;
	std::vector<T> tree;
	CombineOp combinator;
	UpdateOp updater;

  public:
	explicit segment_tree(const std::vector<T> &init, CombineOp combinator = {},
	                      UpdateOp updater = {})
	    : n(init.size()), tree(n), combinator(combinator), updater(updater) {
		std::copy(init.begin(), init.end(), back_inserter(tree));
		for (size_t i = n - 1; i > 0; i--)
			tree[i] = combinator(tree[i << 1], tree[i << 1 | 1]);
	}
	explicit segment_tree(int n, const T &init = {}, CombineOp combinator = {},
	                      UpdateOp updater = {})
	    : segment_tree(std::vector<T>(n, init), combinator, updater) {}

	size_t size() const { return n; }

	void modify(size_t p, const U &val) {
		p += n;
		tree[p] = updater(tree[p], val);
		while (p >>= 1)
			tree[p] = combinator(tree[p << 1], tree[p << 1 | 1]);
	}

	T query(size_t l, size_t r) const {
		std::optional<T> resl, resr;
		for (l += n, r += n + 1; l < r; l >>= 1, r >>= 1) {
			if (l & 1)
				resl =
				    resl.has_value() ? combinator(*resl, tree[l++]) : tree[l++];
			if (r & 1)
				resr =
				    resr.has_value() ? combinator(tree[--r], *resr) : tree[--r];
		}
		if (!resl.has_value())
			return *resr;
		if (!resr.has_value())
			return *resl;
		return combinator(*resl, *resr);
	}
};

template <class T, class U = T, class CombineOp = std::plus<>,
          class UpdateOp = std::plus<>, class CombineUpdateOp = UpdateOp,
          class UpdateLenOp = fn::noop>
class lazy_segment_tree {
	size_t n, h;
	std::vector<T> tree;
	std::vector<std::optional<U>> lazy;
	CombineOp combinator;
	UpdateOp updater;
	CombineUpdateOp lazyCombinator;
	UpdateLenOp updaterLen;

	void calc(size_t p, size_t len) {
		tree[p] = combinator(tree[p << 1], tree[p << 1 | 1]);
		if (lazy[p]) {
			tree[p] = updater(tree[p], updaterLen(*lazy[p], len));
		}
	}

	void apply(size_t p, const U &val, size_t len) {
		tree[p] = updater(tree[p], updaterLen(val, len));
		if (p < n)
			lazy[p] = lazy[p].has_value() ? lazyCombinator(*lazy[p], val) : val;
	}

	void build(size_t p) {
		int len = 2;
		for (p += n; p >>= 1; len <<= 1)
			calc(p, len);
	}

	void push(size_t p) {
		int s = h, len = 1 << (h - 1);
		for (p += n; s > 0; s--, len >>= 1) {
			int i = p >> s;
			if (lazy[i]) {
				apply(i << 1, *lazy[i], len);
				apply(i << 1 | 1, *lazy[i], len);
				lazy[i].reset();
			}
		}
	}

  public:
	explicit lazy_segment_tree(const std::vector<T> &init,
	                           CombineOp combinator = {}, UpdateOp updater = {},
	                           CombineUpdateOp lazyCombinator = {},
	                           UpdateLenOp updaterLen = {})
	    : n(init.size()), h(std::bit_width(n)), tree(n), lazy(n + n),
	      combinator(combinator), updater(updater),
	      lazyCombinator(lazyCombinator), updaterLen(updaterLen) {
		copy(init.begin(), init.end(), back_inserter(tree));
		for (int i = n - 1; i > 0; i--)
			tree[i] = combinator(tree[i << 1], tree[i << 1 | 1]);
	}
	explicit lazy_segment_tree(size_t n, const T &init = {},
	                           CombineOp combinator = {}, UpdateOp updater = {},
	                           CombineUpdateOp lazyCombinator = {},
	                           UpdateLenOp updaterLen = {})
	    : lazy_segment_tree(std::vector<T>(n, init), combinator, updater,
	                        lazyCombinator, updaterLen) {}

	void modify(int l, int r, const U &val) {
		push(l);
		push(r);
		int l0 = l, r0 = r, len = 1;
		for (l += n, r += n + 1; l < r; l >>= 1, r >>= 1, len <<= 1) {
			if (l & 1)
				apply(l++, val, len);
			if (r & 1)
				apply(--r, val, len);
		}
		build(l0);
		build(r0);
	}

	T query(int l, int r) {
		push(l);
		push(r);
		std::optional<T> resl, resr;
		for (l += n, r += n + 1; l < r; l >>= 1, r >>= 1) {
			if (l & 1)
				resl =
				    resl.has_value() ? combinator(*resl, tree[l++]) : tree[l++];
			if (r & 1)
				resr =
				    resr.has_value() ? combinator(tree[--r], *resr) : tree[--r];
		}
		if (!resl.has_value())
			return *resr;
		if (!resr.has_value())
			return *resl;
		return combinator(*resl, *resr);
	}
};

#endif
