/**
 * @file utilities.hpp
 * @brief Provides utility functions and data structures such as prefix sum.
 */

#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <algorithm>
#include <functional>
#include <memory>
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

/*
 * Weird patch to make vector push_back usable in data structures in this
 * template repository. The copy constructor is not available to prevent
 * accidental costly copy operations.
 */
template <class T> struct magic_vector {
	std::unique_ptr<std::vector<T>> ptr;

	magic_vector() : ptr(new std::vector<T>()) {}
	magic_vector(size_t n) : ptr(new std::vector<T>(n)) {}
	magic_vector(size_t n, const T &x) : ptr(new std::vector<T>(n, x)) {}
	magic_vector(magic_vector &&other) = default;
	magic_vector &operator=(magic_vector &&other) = default;

	magic_vector &&operator+(size_t x) {
		ptr->push_back(x);
		return std::move(*this);
	}
	magic_vector &&operator+(magic_vector &&other) {
		if (size() < other.size()) {
			ptr.swap(other.ptr);
		}
		std::copy(other.ptr->begin(), other.ptr->end(),
		          std::back_inserter(*ptr));
	}
	std::vector<T> &operator*() { return *this; }

	T &operator[](size_t i) { return ptr->at(i); }
	T &front() { return ptr->front(); }
	const T &front() const { return ptr->front(); }
	T &back() { return ptr->back(); }
	const T &back() const { return ptr->back(); }
	auto begin() { return ptr->begin(); }
	auto begin() const { return ptr->begin(); }
	auto end() { return ptr->end(); }
	auto end() const { return ptr->end(); }
	auto empty() const { return ptr->empty(); }
	auto size() const { return ptr->size(); }
	void clear() { ptr->clear(); }
	template <class Arg> void push_back(Arg &&x) {
		ptr->push_back(std::forward<Arg>(x));
	}
	template <class... Args> void emplace_back(Args &&...args) {
		ptr->emplace_back(std::forward<Args>(args)...);
	}
};

template <class T> class vector_rollback {
	std::vector<T> data;
	std::vector<std::pair<size_t, T>> history;

  public:
	static constexpr size_t last_version = -1;

	template <class... Args>
	vector_rollback(Args... args) : data(args...), history() {}
	size_t size() const { return data.size(); }
	size_t version() const { return history.size(); }
	T modify(size_t i, T x) {
		history.emplace_back(i, data[i]);
		data[i] = x;
		return x;
	}
	const T &front() const { return data.front(); }
	const T &back() const { return data.back(); }
	const T &operator[](size_t i) const { return data[i]; }
	const T &at(size_t i) const { return data.at(i); }
	void rollback(size_t version = last_version) {
		if (version == last_version) {
			if (history.empty())
				return;
			version = history.size() - 1;
		}
		while (history.size() > version) {
			auto [i, x] = history.back();
			history.pop_back();
			data[i] = x;
		}
	}
};

#endif
