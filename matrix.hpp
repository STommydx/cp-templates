/**
 * @file matrix.hpp
 * @brief Provides math matrix operations
 */

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <valarray>

template <class T> class matrix;

#define NON_MEMBER_BINARY_OP(OP)                                               \
	template <class T>                                                         \
	matrix<T> operator OP(const matrix<T> &a, const matrix<T> &b);             \
	template <class T> matrix<T> operator OP(const matrix<T> &a, const T & b); \
	template <class T> matrix<T> operator OP(const T & a, const matrix<T> &b);

NON_MEMBER_BINARY_OP(+)
NON_MEMBER_BINARY_OP(-)
NON_MEMBER_BINARY_OP(*)
NON_MEMBER_BINARY_OP(/)
NON_MEMBER_BINARY_OP(%)
NON_MEMBER_BINARY_OP(&)
NON_MEMBER_BINARY_OP(|)
NON_MEMBER_BINARY_OP(^)
NON_MEMBER_BINARY_OP(<<)
NON_MEMBER_BINARY_OP(>>)
#undef NON_MEMBER_BINARY_OP

#define NON_MEMBER_BINARY_PREDICATE(OP)                                        \
	template <class T>                                                         \
	matrix<bool> operator OP(const matrix<T> &a, const matrix<T> &b);          \
	template <class T>                                                         \
	matrix<bool> operator OP(const matrix<T> &a, const T & b);                 \
	template <class T>                                                         \
	matrix<bool> operator OP(const T & a, const matrix<T> &b);

NON_MEMBER_BINARY_PREDICATE(&&)
NON_MEMBER_BINARY_PREDICATE(||)
NON_MEMBER_BINARY_PREDICATE(==)
NON_MEMBER_BINARY_PREDICATE(!=)
NON_MEMBER_BINARY_PREDICATE(<)
NON_MEMBER_BINARY_PREDICATE(>)
NON_MEMBER_BINARY_PREDICATE(<=)
NON_MEMBER_BINARY_PREDICATE(>=)
#undef NON_MEMBER_BINARY_PREDICATE

template <class T> matrix<T> matmul(const matrix<T> &a, const matrix<T> &b);

template <class T> class matrix {
	size_t n, m;
	std::valarray<T> dat;

  public:
	static constexpr size_t none_axis = -1;

	/*
	 * Constructors and assignment operators
	 */
	explicit matrix(size_t count_n, size_t count_m, const T &val = {})
	    : n(count_n), m(count_m), dat(val, count_n * count_m) {}
	explicit matrix(size_t count_n, size_t count_m,
	                const std::valarray<T> &vals)
	    : n(count_n), m(count_m), dat(vals) {}
	explicit matrix(size_t count_n, size_t count_m, std::valarray<T> &&vals)
	    : n(count_n), m(count_m), dat(std::move(vals)) {}
	matrix(const std::vector<std::vector<T>> &v)
	    : n(v.size()), m(v.empty() ? 0 : v[0].size()), dat(n * m) {
		for (size_t i = 0; i < n; ++i) {
			std::ranges::copy(v[i], dat.begin() + i * m);
		}
	}

	matrix &operator=(const std::valarray<T> &other) {
		dat = other;
		return *this;
	}
	matrix &operator=(const std::valarray<T> &&other) {
		dat = std::move(other);
		return *this;
	}
	matrix &operator=(const T &val) {
		dat = val;
		return *this;
	}

	/*
	 * Other matrix builders
	 */
	static matrix zeros(size_t count_n, size_t count_m) {
		return matrix(count_n, count_m, 0);
	}
	static matrix ones(size_t count_n, size_t count_m) {
		return matrix(count_n, count_m, 1);
	}
	static matrix eye(size_t count_n, size_t count_m = none_axis,
	                  size_t k = 0) {
		if (count_m == none_axis) {
			count_m = count_n;
		}
		matrix res(count_n, count_m, 0);
		res.diagonal(k) = 1;
		return res;
	}
	static matrix identity(size_t count_n) { return eye(count_n); }

	/*
	 * Element access
	 */
	const T &at(size_t i, size_t j) const { return dat[i * m + j]; }
	T &at(size_t i, size_t j) { return dat[i * m + j]; }
	std::valarray<T> row(size_t i) const {
		return dat[std::slice(i * m, m, 1)];
	}
	std::slice_array<T> row(size_t i) { return dat[std::slice(i * m, m, 1)]; }
	std::valarray<T> col(size_t j) const { return dat[std::slice(j, n, m)]; }
	std::slice_array<T> col(size_t j) { return dat[std::slice(j, n, m)]; }
	std::valarray<T> diagonal(size_t offset) const {
		return dat[std::slice(offset, std::min(n, m - offset), m + 1)];
	}
	std::slice_array<T> diagonal(size_t offset) {
		return dat[std::slice(offset, std::min(n, m - offset), m + 1)];
	}
	matrix transpose() const {
		matrix res(m, n);
		for (size_t i = 0; i < n; ++i)
			res.col(i) = row(i);
		return res;
	}
	const std::valarray<T> &data() const { return dat; }
	std::valarray<T> &data() { return dat; }
	std::valarray<T> flatten() const { return dat; }
	std::vector<std::vector<T>> to_vector() const {
		std::vector<std::vector<T>> res(n, std::vector<T>(m));
		for (size_t i = 0; i < n; ++i) {
			std::ranges::copy(row(i), res[i].begin());
		}
		return res;
	}

	/*
	 * Metadata
	 */
	size_t size() const { return n * m; }
	std::pair<size_t, size_t> shape() const { return {n, m}; }

	/*
	 * Aggregate operations
	 */
	template <size_t Axis, bool KeepDims>
	using aggregate_t = typename std::conditional_t<
	    KeepDims, matrix,
	    std::conditional_t<Axis == none_axis, T, std::valarray<T>>>;
	template <size_t Axis = none_axis, bool KeepDims = false,
	          std::invocable<const std::valarray<T> &> Aggregate>
	aggregate_t<Axis, KeepDims> aggregate(Aggregate f = {}) const {
		// note that partial function templates specialization is not allowed
		if constexpr (KeepDims) {
			if constexpr (Axis == none_axis) {
				return matrix(1, 1, std::invoke(f, dat));
			} else if constexpr (Axis == 0) {
				matrix ret(1, m);
				for (size_t i = 0; i < m; ++i) {
					ret(0, i) = std::invoke(f, col(i));
				}
				return ret;
			} else if constexpr (Axis == 1) {
				matrix ret(n, 1);
				for (size_t i = 0; i < n; ++i) {
					ret(i, 0) = std::invoke(f, row(i));
				}
				return ret;
			} else {
				return *this;
			}
		} else {
			if constexpr (Axis == none_axis) {
				return std::invoke(f, dat);
			} else if constexpr (Axis == 0) {
				std::valarray<T> ret(m);
				for (size_t i = 0; i < m; ++i) {
					ret[i] = std::invoke(f, col(i));
				}
				return ret;
			} else if constexpr (Axis == 1) {
				std::valarray<T> ret(n);
				for (size_t i = 0; i < n; ++i) {
					ret[i] = std::invoke(f, row(i));
				}
				return ret;
			} else {
				return dat;
			}
		}
	}

	template <size_t Axis = none_axis, bool KeepDims = false>
	aggregate_t<Axis, KeepDims> sum() const {
		return aggregate<Axis, KeepDims>(&std::valarray<T>::sum);
	}
	template <size_t Axis = none_axis, bool KeepDims = false>
	aggregate_t<Axis, KeepDims> min() const {
		return aggregate<Axis, KeepDims>(&std::valarray<T>::min);
	}
	template <size_t Axis = none_axis, bool KeepDims = false>
	aggregate_t<Axis, KeepDims> max() const {
		return aggregate<Axis, KeepDims>(&std::valarray<T>::max);
	}

	/*
	 * Operator overloads
	 */
	std::valarray<T> operator[](size_t i) const { return row(i); }
	std::slice_array<T> operator[](size_t i) { return row(i); }
	const T &operator()(size_t i, size_t j) const { return at(i, j); }
	T &operator()(size_t i, size_t j) { return at(i, j); }
	operator std::vector<std::vector<T>>() const { return to_vector(); }

	matrix operator+() const { return matrix(n, m, +dat); }
	matrix operator-() const { return matrix(n, m, -dat); }
	matrix operator~() const { return matrix(n, m, ~dat); }
	matrix<bool> operator!() const { return matrix<bool>(n, m, !dat); }

#define MEMBER_BINARY_OP(OP)                                                   \
	matrix &operator OP(const matrix & m) {                                    \
		dat OP m.dat;                                                          \
		return *this;                                                          \
	}                                                                          \
	matrix &operator OP(const T & x) {                                         \
		dat OP x;                                                              \
		return *this;                                                          \
	}

	MEMBER_BINARY_OP(+=)
	MEMBER_BINARY_OP(-=)
	MEMBER_BINARY_OP(*=)
	MEMBER_BINARY_OP(/=)
	MEMBER_BINARY_OP(%=)
	MEMBER_BINARY_OP(&=)
	MEMBER_BINARY_OP(|=)
	MEMBER_BINARY_OP(^=)
	MEMBER_BINARY_OP(<<=)
	MEMBER_BINARY_OP(>>=)
#undef MEMBER_BINARY_OP

/*
 * Non-member functions
 */
#define NON_MEMBER_BINARY_OP(OP)                                               \
	friend matrix<T> operator OP<>(const matrix<T> &a, const matrix<T> &b);    \
	friend matrix<T> operator OP<>(const matrix<T> &a, const T & b);           \
	friend matrix<T> operator OP<>(const T & a, const matrix<T> &b);

	NON_MEMBER_BINARY_OP(+)
	NON_MEMBER_BINARY_OP(-)
	NON_MEMBER_BINARY_OP(*)
	NON_MEMBER_BINARY_OP(/)
	NON_MEMBER_BINARY_OP(%)
	NON_MEMBER_BINARY_OP(&)
	NON_MEMBER_BINARY_OP(|)
	NON_MEMBER_BINARY_OP(^)
	NON_MEMBER_BINARY_OP(<<)
	NON_MEMBER_BINARY_OP(>>)
#undef NON_MEMBER_BINARY_OP

#define NON_MEMBER_BINARY_PREDICATE(OP)                                        \
	friend matrix<bool> operator OP<>(const matrix<T> &a, const matrix<T> &b); \
	friend matrix<bool> operator OP<>(const matrix<T> &a, const T & b);        \
	friend matrix<bool> operator OP<>(const T & a, const matrix<T> &b);

	NON_MEMBER_BINARY_PREDICATE(&&)
	NON_MEMBER_BINARY_PREDICATE(||)
	NON_MEMBER_BINARY_PREDICATE(==)
	NON_MEMBER_BINARY_PREDICATE(!=)
	NON_MEMBER_BINARY_PREDICATE(<)
	NON_MEMBER_BINARY_PREDICATE(<=)
	NON_MEMBER_BINARY_PREDICATE(>)
	NON_MEMBER_BINARY_PREDICATE(>=)
#undef NON_MEMBER_BINARY_PREDICATE

	friend matrix matmul<>(const matrix &a, const matrix &b);
};

#define NON_MEMBER_BINARY_OP(OP)                                               \
	template <class T>                                                         \
	matrix<T> operator OP(const matrix<T> &a, const matrix<T> &b) {            \
		return matrix<T>(a.n, a.m, a.dat OP b.dat);                            \
	}                                                                          \
	template <class T>                                                         \
	matrix<T> operator OP(const matrix<T> &a, const T & b) {                   \
		return matrix<T>(a.n, a.m, a.dat OP b);                                \
	}                                                                          \
	template <class T>                                                         \
	matrix<T> operator OP(const T & a, const matrix<T> &b) {                   \
		return matrix<T>(b.n, b.m, a OP b.dat);                                \
	}

NON_MEMBER_BINARY_OP(+)
NON_MEMBER_BINARY_OP(-)
NON_MEMBER_BINARY_OP(*)
NON_MEMBER_BINARY_OP(/)
NON_MEMBER_BINARY_OP(%)
NON_MEMBER_BINARY_OP(&)
NON_MEMBER_BINARY_OP(|)
NON_MEMBER_BINARY_OP(^)
NON_MEMBER_BINARY_OP(<<)
NON_MEMBER_BINARY_OP(>>)
#undef NON_MEMBER_BINARY_OP

#define NON_MEMBER_BINARY_PREDICATE(OP)                                        \
	template <class T>                                                         \
	matrix<bool> operator OP(const matrix<T> &a, const matrix<T> &b) {         \
		return matrix<bool>(a.n, a.m, a.dat OP b.dat);                         \
	}                                                                          \
	template <class T>                                                         \
	matrix<bool> operator OP(const matrix<T> &a, const T & b) {                \
		return matrix<bool>(a.n, a.m, a.dat OP b);                             \
	}                                                                          \
	template <class T>                                                         \
	matrix<bool> operator OP(const T & a, const matrix<T> &b) {                \
		return matrix<bool>(b.n, b.m, a OP b.dat);                             \
	}

NON_MEMBER_BINARY_PREDICATE(&&)
NON_MEMBER_BINARY_PREDICATE(||)
NON_MEMBER_BINARY_PREDICATE(==)
NON_MEMBER_BINARY_PREDICATE(!=)
NON_MEMBER_BINARY_PREDICATE(<)
NON_MEMBER_BINARY_PREDICATE(<=)
NON_MEMBER_BINARY_PREDICATE(>)
NON_MEMBER_BINARY_PREDICATE(>=)
#undef NON_MEMBER_BINARY_PREDICATE

template <class T> matrix<T> matmul(const matrix<T> &a, const matrix<T> &b) {
	matrix<T> result(a.n, b.m);
	for (size_t i = 0; i < a.n; ++i) {
		for (size_t j = 0; j < b.m; ++j) {
			result.at(i, j) = (a.row(i) * b.col(j)).sum();
		}
	}
	return result;
}

#endif
