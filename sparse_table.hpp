/**
 * @file sparse_table.hpp
 * @brief Sparse table data structure implementation
 */

#ifndef SPARSE_TABLE_HPP
#define SPARSE_TABLE_HPP

#include <bit>
#include <vector>

template <class T, class Op = std::bit_or<>> class sparse_table {
	size_t n, m;
	std::vector<std::vector<T>> dp;
	Op op;

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

	T query(size_t l, size_t r) const {
		int j = std::bit_width(r - l + 1) - 1;
		return op(dp[j][l], dp[j][r + 1 - (1 << j)]);
	}
};

template <class T, class Op = std::bit_or<>>
class sparse_table_matrix {
    size_t nx, ny, mx;
    std::vector<std::vector<sparse_table<T, Op>>> dp;
    Op op;

    public:
    sparse_table_matrix(const std::vector<std::vector<T>> &init, const Op &comb = {})
        : nx(init.size()), ny(init[0].size()), mx(std::bit_width(nx)), dp(mx), op(comb) {
            for (size_t i = 0; i < nx; i++) {
                dp[0].emplace_back(init[i], comb);
            }
            for (size_t j = 1; j < mx; j++) {
                for (size_t i = 0; i + (1 << j) <= nx; i++) {
                    std::vector<T> w(ny);
                    for (size_t k = 0; k < ny; k++) {
                        w[k] = op(dp[j - 1][i].query(k, k), dp[j - 1][i + (1 << (j - 1))].query(k, k));
                    }
                    dp[j].emplace_back(w, comb);
                }
            }
        }

    T query(size_t lx, size_t ly, size_t rx, size_t ry) const {
        int j = std::bit_width(rx - lx + 1) - 1;
        return op(dp[j][lx].query(ly, ry), dp[j][rx + 1 - (1 << j)].query(ly, ry));
    }
};

#endif
