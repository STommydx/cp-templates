/**
 * @file fenwick.hpp
 * @brief Fenwick tree data structure implementation
 */

#ifndef FENWICK_HPP
#define FENWICK_HPP

#include <bit>
#include <iostream>
#include <vector>

template <class T> class fenwick {
	std::vector<T> ft;
	T _query(size_t idx) const {
		T sum{};
		for (; idx > 0; idx -= idx & (~idx + 1))
			sum += ft[idx - 1];
		return sum;
	}
	void _modify(size_t idx, const T &val) {
		for (; idx <= ft.size(); idx += idx & (~idx + 1))
			ft[idx - 1] += val;
	}

  public:
	explicit fenwick(size_t n) : ft(n) {}
	explicit fenwick(const std::vector<T> &v) : fenwick(v.size()) {
		for (size_t i = 0; i < v.size(); ++i)
			modify(i, v[i]);
	}
	void modify(size_t idx, const T &val) { _modify(idx + 1, val); }
	T query(size_t idx) const { return _query(idx + 1); }
	T query(size_t l, size_t r) const {
		if (l == 0)
			return query(r);
		return query(r) - query(l - 1);
	}
};

#endif