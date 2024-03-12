/**
 * @file sparse_table.hpp
 * @brief Sparse table data structure implementation
 */

#ifndef SPARSE_TABLE_HPP
#define SPARSE_TABLE_HPP

#include <iostream>
#include <vector>

template <class T, class Op = std::bit_or<>> class sparse_table {
	Op op;
	size_t n, m;
	std::vector<std::vector<T>> dp;

  public:
	sparse_table(const std::vector<T> &init, const Op &comb = {})
	    : n(init.size()), m(std::bit_width(n)), dp(m, std::vector<T>(n)),
	      op(comb) {
		dp[0] = init;
		for (size_t j = 1; j < m; j++) {
			for (size_t i = 0; i + (1 << j) <= n; i++) {
				dp[j][i] = op(dp[j - 1][i], dp[j - 1][i + (1 << (j - 1))]);
			}
		}
	}

	T query(size_t l, size_t r) {
		int j = std::bit_width(r - l + 1) - 1;
		return op(dp[j][l], dp[j][r + 1 - (1 << j)]);
	}
};

#endif
