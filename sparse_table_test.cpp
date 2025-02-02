#include "functional.hpp"
#include "sparse_table.hpp"

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <numeric>

TEST_CASE("sparse_table with bitwise or", "[sparse_table]") {
	std::vector<int> v{3, 2, 8, 5};
	sparse_table st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 15);
	REQUIRE(st.query(2, 3) == 13);
}

TEST_CASE("sparse_table with minimum", "[sparse_table]") {
	std::vector<int> v{3, 2, 8, 5};
	sparse_table<int, fn::minimum<>> st(v);
	REQUIRE(st.query(0, 1) == 2);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(2, 3) == 5);
}

TEST_CASE("sparse_table with minimum index", "[sparse_table]") {
	std::vector<int> v{3, 2, 8, 5};
	std::vector<int> p(v.size());
	std::iota(p.begin(), p.end(), 0);
	fn::minimum min_obj({}, [&](int idx) { return v[idx]; });
	sparse_table st(p, min_obj);
	REQUIRE(st.query(0, 1) == 1);
	REQUIRE(st.query(1, 3) == 1);
	REQUIRE(st.query(2, 3) == 3);
}

TEST_CASE("sparse_table matrix with bitwise or", "[sparse_table_matrix]") {
	std::vector<std::vector<int>> v{{3, 2, 8, 5}, {1, 2, 3, 4}, {5, 16, 7, 8}};
	sparse_table_matrix st(v);
	REQUIRE(st.query(0, 0, 0, 2) == 11);
	REQUIRE(st.query(0, 0, 1, 1) == 3);
	REQUIRE(st.query(1, 1, 1, 3) == 7);
	REQUIRE(st.query(0, 1, 2, 3) == 31);
}

TEST_CASE("sparse_table matrix with minimum and maximum",
          "[sparse_table_matrix]") {
	std::vector<std::vector<int>> v{{3, 2, 8, 5}, {1, 2, 3, 4}, {5, 16, 7, 8}};
	sparse_table_matrix<int, fn::minimum<>> st(v);
	REQUIRE(st.query(0, 0, 0, 2) == 2);
	REQUIRE(st.query(0, 0, 1, 1) == 1);
	REQUIRE(st.query(1, 1, 1, 3) == 2);
	REQUIRE(st.query(0, 1, 2, 3) == 2);
	sparse_table_matrix<int, fn::maximum<>> st_mx(v);
	REQUIRE(st_mx.query(0, 0, 0, 2) == 8);
	REQUIRE(st_mx.query(0, 0, 1, 1) == 3);
	REQUIRE(st_mx.query(1, 1, 1, 3) == 4);
	REQUIRE(st_mx.query(0, 1, 2, 3) == 16);
}

TEST_CASE("sparse_table matrix with projection", "[sparse_table_matrix]") {
	std::vector<std::vector<int>> v{{3, 2, 8, 5}, {1, 2, 3, 4}, {5, 16, 7, 8}};
	sparse_table_matrix<int, fn::minimum<int, decltype([](int x) {
		                                     return x % 2 == 0 ? x : -x;
	                                     })>>
	    st(v);
	REQUIRE(st.query(0, 0, 0, 2) == 3);
	REQUIRE(st.query(0, 0, 1, 1) == 3);
	REQUIRE(st.query(1, 1, 1, 3) == 3);
	REQUIRE(st.query(0, 1, 2, 3) == 7);
}
