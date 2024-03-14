/**
 * @file utilities.hpp
 * @brief Provides utility functions and data structures such as prefix sum.
 */

#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <vector>

/**
 * prefix_sum is a data structure that can answer range sum queries in O(1)
 * time.
 */
template <class T> class prefix_sum : public std::vector<T> {
  public:
	explicit prefix_sum(const std::vector<T> &v) : std::vector<T>(v) {
		partial_sum(begin(v), end(v), begin(*this));
	}
	T query(size_t l, size_t r) const {
		if (l == 0) {
			return (*this)[r];
		}
		return (*this)[r] - (*this)[l - 1];
	}
};

/**
 * prefix_sum_matrix is a data structure that can answer 2D range sum queries in
 * O(1) time.
 */
template <class T>
class prefix_sum_matrix : public std::vector<std::vector<T>> {
  public:
	explicit prefix_sum_matrix(const std::vector<std::vector<T>> &v)
	    : std::vector<std::vector<T>>(v) {
		auto &a = *this;
		for (size_t i = 0; i < size(a); i++)
			for (size_t j = 0; j < size(a[0]); j++) {
				if (i > 0)
					a[i][j] += a[i - 1][j];
				if (j > 0)
					a[i][j] += a[i][j - 1];
				if (i > 0 && j > 0)
					a[i][j] -= a[i - 1][j - 1];
			}
	}
	T query(size_t lx, size_t ly, size_t rx, size_t ry) const {
		auto &a = *this;
		T sum = a[rx][ry];
		if (lx > 0)
			sum -= a[lx - 1][ry];
		if (ly > 0)
			sum -= a[rx][ly - 1];
		if (lx > 0 && ly > 0)
			sum += a[lx - 1][ly - 1];
		return sum;
	}
};

/**
 * compression_vector is a data structure that performs coordinate compression.
 */
template <class T, class SortCompare = std::ranges::less,
          class UniqueCompare = std::ranges::equal_to,
          class Proj = std::identity>
class compression_vector : public std::vector<T> {
	SortCompare sort_compare;
	UniqueCompare unique_compare;
	Proj proj;

  public:
	explicit compression_vector(SortCompare sort_compare = {},
	                            UniqueCompare unique_compare = {},
	                            Proj proj = {})
	    : std::vector<T>(), sort_compare(sort_compare),
	      unique_compare(unique_compare), proj(proj) {}
	explicit compression_vector(const std::vector<T> &v,
	                            SortCompare sort_compare = {},
	                            UniqueCompare unique_compare = {},
	                            Proj proj = {})
	    : std::vector<T>(v), sort_compare(sort_compare),
	      unique_compare(unique_compare), proj(proj) {}
	explicit compression_vector(size_t count, SortCompare sort_compare = {},
	                            UniqueCompare unique_compare = {},
	                            Proj proj = {})
	    : std::vector<T>(count), sort_compare(sort_compare),
	      unique_compare(unique_compare), proj(proj) {}
	explicit compression_vector(size_t count, const T &value,
	                            SortCompare sort_compare = {},
	                            UniqueCompare unique_compare = {},
	                            Proj proj = {})
	    : std::vector<T>(count, value), sort_compare(sort_compare),
	      unique_compare(unique_compare), proj(proj) {}
	template <class Iter>
	explicit compression_vector(Iter first, Iter last,
	                            SortCompare sort_compare = {},
	                            UniqueCompare unique_compare = {},
	                            Proj proj = {})
	    : std::vector<T>(first, last), sort_compare(sort_compare),
	      unique_compare(unique_compare), proj(proj) {}

	void compress() {
		std::ranges::sort(*this, sort_compare, proj);
		auto past_the_end_range =
		    std::ranges::unique(*this, unique_compare, proj);
		this->erase(std::ranges::begin(past_the_end_range),
		            std::ranges::end(past_the_end_range));
	}

	auto lower_bound(const T &x) const {
		return std::ranges::lower_bound(*this, x, sort_compare, proj);
	}

	auto upper_bound(const T &x) const {
		return std::ranges::upper_bound(*this, x, sort_compare, proj);
	}

	auto equal_range(const T &x) const {
		return std::ranges::equal_range(*this, x, sort_compare, proj);
	}

	size_t lower_bound_index(const T &x) const {
		return std::ranges::distance(std::ranges::begin(*this), lower_bound(x));
	}

	size_t upper_bound_index(const T &x) const {
		return std::ranges::distance(std::ranges::begin(*this), upper_bound(x));
	}

	std::pair<size_t, size_t> equal_range_index(const T &x) const {
		auto [l, r] = equal_range(x);
		return {std::ranges::distance(std::ranges::begin(*this), l),
		        std::ranges::distance(std::ranges::begin(*this), r)};
	}
};
template <class Iter, class SortCompare, class UniqueCompare, class Proj>
compression_vector(Iter, Iter, SortCompare, UniqueCompare, Proj)
    -> compression_vector<std::iter_value_t<Iter>, SortCompare, UniqueCompare,
                          Proj>;

template <int N> class prime_sieve {
	std::array<int, N> min_prime_factor;

  public:
	constexpr prime_sieve() {
		min_prime_factor.fill(0);
		for (int i = 2; i < N; i++) {
			if (!min_prime_factor[i]) {
				min_prime_factor[i] = i;
				if (i * i >= N)
					continue;
				for (int j = i * i; j < N; j += i)
					if (!min_prime_factor[j])
						min_prime_factor[j] = i;
			}
		}
	}
	constexpr int operator[](int n) const { return min_prime_factor[n]; }
	constexpr bool is_prime(int n) const { return min_prime_factor[n] == n; }
};

#endif
