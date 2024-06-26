#include "matrix.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <utility>

TEST_CASE("matrix construction and member functions", "[matrix]") {
	matrix<int> a(2, 2, 1), b(2, 2, 3);
	SECTION("matrix construction and element access") {
		a.at(0, 0) = 2;
		REQUIRE(a(0, 0) == 2);
		REQUIRE(a(0, 1) == 1);
		REQUIRE(a(1, 0) == 1);
		REQUIRE(a(1, 1) == 1);
		a[0] = 3;
		REQUIRE(a(0, 0) == 3);
		REQUIRE(a(0, 1) == 3);
		REQUIRE(a(1, 0) == 1);
		REQUIRE(a(1, 1) == 1);
		a.col(1) = 4;
		REQUIRE(a(0, 0) == 3);
		REQUIRE(a(0, 1) == 4);
		REQUIRE(a(1, 0) == 1);
		REQUIRE(a(1, 1) == 4);
		a.row(1) = a.col(1);
		REQUIRE(a(0, 0) == 3);
		REQUIRE(a(0, 1) == 4);
		REQUIRE(a(1, 0) == 4);
		REQUIRE(a(1, 1) == 4);
		a(0, 1) = 2;
		matrix<int> c = a.transpose();
		REQUIRE(c(0, 0) == 3);
		REQUIRE(c(0, 1) == 4);
		REQUIRE(c(1, 0) == 2);
		REQUIRE(c(1, 1) == 4);
	}
	SECTION("submatrix and concatenation") {
		matrix<int> c = a.concatenate<0>(b);
		REQUIRE(c.shape() == std::pair<size_t, size_t>(4, 2));
		REQUIRE(c(1, 1) == 1);
		REQUIRE(c(2, 1) == 3);
		matrix<int> d = a.concatenate<1>(b);
		REQUIRE(d.shape() == std::pair<size_t, size_t>(2, 4));
		REQUIRE(d(1, 1) == 1);
		REQUIRE(d(1, 3) == 3);
		matrix<int> e = c.concatenate<1>(d.transpose());
		REQUIRE(e.shape() == std::pair<size_t, size_t>(4, 4));
		matrix<int> f = e.submatrix(1, 1, 3, 2);
		REQUIRE(f.shape() == std::pair<size_t, size_t>(3, 2));
		REQUIRE(f(0, 0) == 1);
		REQUIRE(f(0, 1) == 1);
		REQUIRE(f(1, 0) == 3);
		REQUIRE(f(1, 1) == 3);
		REQUIRE(f(2, 0) == 3);
		REQUIRE(f(2, 1) == 3);
	}
	SECTION("matrix construction functions") {
		matrix<int> c = matrix<int>::eye(2, 3);
		REQUIRE(c(0, 0) == 1);
		REQUIRE(c(0, 1) == 0);
		REQUIRE(c(0, 2) == 0);
		REQUIRE(c(1, 0) == 0);
		REQUIRE(c(1, 1) == 1);
		REQUIRE(c(1, 2) == 0);
		matrix<int> d = matrix<int>::eye(3, 2, 1);
		REQUIRE(d(0, 0) == 0);
		REQUIRE(d(0, 1) == 1);
		REQUIRE(d(1, 0) == 0);
		REQUIRE(d(1, 1) == 0);
		REQUIRE(d(2, 0) == 0);
		REQUIRE(d(2, 1) == 0);
	}
	SECTION("aggregate functions") {
		a(0, 0) = 2;
		a(0, 1) = 0;
		REQUIRE(a.sum() == 4);
		REQUIRE(std::ranges::equal(a.sum<0>(), std::vector<int>{3, 1}));
		REQUIRE(std::ranges::equal(a.sum<1>(), std::vector<int>{2, 2}));
		auto aggregated = a.sum<0, true>();
		REQUIRE(aggregated.shape() == std::pair<size_t, size_t>(1, 2));
		REQUIRE(std::ranges::equal(std::valarray<int>(aggregated.row(0)),
		                           std::vector<int>{3, 1}));
		REQUIRE(a.max() == 2);
		REQUIRE(a.min() == 0);
	}
	SECTION("operator overloads") {
		a += b;
		REQUIRE(a(0, 0) == 4);
		a -= b;
		REQUIRE(a(0, 0) == 1);
		a *= b;
		REQUIRE(a(0, 0) == 3);
		a /= b;
		REQUIRE(a(0, 0) == 1);
		a %= b;
		REQUIRE(a(0, 0) == 1);
		a ^= b;
		REQUIRE(a(0, 0) == 2);
		a |= b;
		REQUIRE(a(0, 0) == 3);
		a &= b;
		REQUIRE(a(0, 0) == 3);
		a <<= b;
		REQUIRE(a(0, 0) == 24);
		a >>= b;
		REQUIRE(a(0, 0) == 3);

		REQUIRE((+a)(0, 0) == 3);
		REQUIRE((-a)(0, 0) == -3);
		REQUIRE((~a)(0, 0) == ~3);
		REQUIRE((!a)(0, 0) == 0);

		a = 1;
		a += 3;
		REQUIRE(a(0, 0) == 4);
		a -= 3;
		REQUIRE(a(0, 0) == 1);
		a *= 3;
		REQUIRE(a(0, 0) == 3);
		a /= 3;
		REQUIRE(a(0, 0) == 1);
		a %= 3;
		REQUIRE(a(0, 0) == 1);
		a ^= 3;
		REQUIRE(a(0, 0) == 2);
		a |= 3;
		REQUIRE(a(0, 0) == 3);
		a &= 3;
		REQUIRE(a(0, 0) == 3);
		a <<= 3;
		REQUIRE(a(0, 0) == 24);
		a >>= 3;
		REQUIRE(a(0, 0) == 3);
	}
	SECTION("std::vector conversion") {
		std::vector<std::vector<int>> v = a;
		REQUIRE(v[0][0] == 1);
		REQUIRE(v[1][0] == 1);
		REQUIRE(v[1][1] == 1);
		REQUIRE(v[0][1] == 1);
		matrix<int> m = v;
		REQUIRE(m(0, 0) == 1);
		REQUIRE(m(1, 0) == 1);
		REQUIRE(m(1, 1) == 1);
		REQUIRE(m(0, 1) == 1);
		std::vector<std::vector<int>> result = matrix<int>(v) + m;
		REQUIRE(result[0][0] == 2);
		REQUIRE(result[1][0] == 2);
		REQUIRE(result[1][1] == 2);
		REQUIRE(result[0][1] == 2);
	}
}

TEST_CASE("matrix arithmetic operator overload", "[matrix]") {
	matrix<int> a(2, 2, 5);
	matrix<int> b(2, 2, 7);

#define BINARY_OP_TEST_CASE(OP)                                                \
	SECTION(#OP) {                                                             \
		auto c = a OP b;                                                       \
		REQUIRE(c(0, 0) == (5 OP 7));                                          \
		auto d = a OP 7;                                                       \
		REQUIRE(d(0, 0) == (5 OP 7));                                          \
		auto e = 7 OP a;                                                       \
		REQUIRE(e(0, 0) == (7 OP 5));                                          \
	}

	BINARY_OP_TEST_CASE(+)
	BINARY_OP_TEST_CASE(-)
	BINARY_OP_TEST_CASE(*)
	BINARY_OP_TEST_CASE(/)
	BINARY_OP_TEST_CASE(%)
	BINARY_OP_TEST_CASE(&)
	BINARY_OP_TEST_CASE(|)
	BINARY_OP_TEST_CASE(^)
	BINARY_OP_TEST_CASE(<<)
	BINARY_OP_TEST_CASE(>>)
	BINARY_OP_TEST_CASE(&&)
	BINARY_OP_TEST_CASE(||)
	BINARY_OP_TEST_CASE(==)
	BINARY_OP_TEST_CASE(!=)
	BINARY_OP_TEST_CASE(<)
	BINARY_OP_TEST_CASE(<=)
	BINARY_OP_TEST_CASE(>)
	BINARY_OP_TEST_CASE(>=)
#undef BINARY_OP_TEST_CASE
}

TEST_CASE("matrix multiplication", "[matrix]") {
	matrix<int> mat(2, 2);
	mat.at(0, 0) = mat.at(0, 1) = mat.at(1, 0) = 1;
	const auto res = matmul(mat, mat);
	REQUIRE(res[0][0] == 2);
	REQUIRE(res[0][1] == 1);
	REQUIRE(res[1][0] == 1);
	REQUIRE(res[1][1] == 1);
}

TEST_CASE("matrix power", "[matrix]") {
	matrix<int> mat(2, 2);
	mat.at(0, 0) = mat.at(0, 1) = mat.at(1, 0) = 1;
	const auto res = matrix_power(mat, 4);
	REQUIRE(res[0][0] == 5);
}

TEST_CASE("gaussian elimination", "[matrix]") {
	SECTION("integers") {
		std::vector<std::vector<int>> mat{
		    {1, 1, 5},
		    {1, 0, 1},
		};
		auto [res, det] = gaussian_elimination<int>(mat);
		REQUIRE(res.shape() == std::pair<size_t, size_t>(2, 3));
		REQUIRE(det == -1);
		REQUIRE(res(0, 0) == 1);
		REQUIRE(res(0, 1) == 0);
		REQUIRE(res(0, 2) == 1);
		REQUIRE(res(1, 0) == 0);
		REQUIRE(res(1, 1) == 1);
		REQUIRE(res(1, 2) == 4);
	}
	SECTION("floating point") {
		std::vector<std::vector<double>> mat{
		    {1, 1.5, 5},
		    {2, 0, 1},
		};
		auto [res, det] = gaussian_elimination<double>(mat);
		REQUIRE(res.shape() == std::pair<size_t, size_t>(2, 3));
		REQUIRE(det == -3);
		REQUIRE(res(0, 0) == 1);
		REQUIRE(res(0, 1) == 0);
		REQUIRE(res(0, 2) == 0.5);
		REQUIRE(res(1, 0) == 0);
		REQUIRE(res(1, 1) == 1);
		REQUIRE(res(1, 2) == 3);
	}
}

TEST_CASE("matrix inverse", "[matrix]") {
	SECTION("integers") {
		std::vector<std::vector<int>> mat{
		    {1, 1},
		    {1, 0},
		};
		auto result = matrix_inverse<int>(mat);
		REQUIRE(result.has_value());
		auto mat_inverse = result.value();
		REQUIRE(mat_inverse.shape() == std::pair<size_t, size_t>(2, 2));
		REQUIRE(mat_inverse(0, 0) == 0);
		REQUIRE(mat_inverse(0, 1) == 1);
		REQUIRE(mat_inverse(1, 0) == 1);
		REQUIRE(mat_inverse(1, 1) == -1);
	}
	SECTION("floating point") {
		std::vector<std::vector<double>> mat{
		    {1, 1},
		    {1, 5},
		};
		auto result = matrix_inverse<double>(mat);
		REQUIRE(result.has_value());
		auto mat_inverse = result.value();
		REQUIRE(mat_inverse.shape() == std::pair<size_t, size_t>(2, 2));
		REQUIRE(mat_inverse(0, 0) == 1.25);
		REQUIRE(mat_inverse(0, 1) == -0.25);
		REQUIRE(mat_inverse(1, 0) == -0.25);
		REQUIRE(mat_inverse(1, 1) == 0.25);
	}
	SECTION("no inverse") {
		std::vector<std::vector<double>> mat{
		    {2, 1},
		    {5, 2.5},
		};
		auto result = matrix_inverse<double>(mat);
		REQUIRE(!result.has_value());
	}
}
