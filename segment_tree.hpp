/**
 * @file segment_tree.hpp
 * @brief Segment tree data structure implementation
 */

#ifndef SEGMENT_TREE_HPP
#define SEGMENT_TREE_HPP

#include <algorithm>
#include <functional>
#include <optional>
#include <vector>

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

#endif
