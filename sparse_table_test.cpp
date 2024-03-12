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
