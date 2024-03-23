#include "utilities.hpp"

#include <catch2/catch_test_macros.hpp>

#include <random>

TEST_CASE("prefix_sum behaves as expected", "[utilities]") {
	std::vector<int> v{3, 2, 4, 5};
	prefix_sum<int> p(v);
	REQUIRE(p.query(0, 0) == 3);
	REQUIRE(p.query(0, 1) == 5);
	REQUIRE(p.query(1, 3) == 11);
}

TEST_CASE("prefix_sum_matrix behaves as expected", "[utilities]") {
	std::vector<std::vector<int>> v{{3, 2, 4}, {5, 6, 7}};
	prefix_sum_matrix<int> p(v);
	REQUIRE(p.query(0, 0, 0, 0) == 3);
	REQUIRE(p.query(0, 0, 0, 1) == 5);
	REQUIRE(p.query(0, 0, 0, 2) == 9);
	REQUIRE(p.query(0, 0, 1, 1) == 16);
	REQUIRE(p.query(1, 0, 1, 2) == 18);
	REQUIRE(p.query(0, 1, 1, 2) == 19);
	REQUIRE(p.query(1, 1, 1, 2) == 13);
}

TEST_CASE("compression_vector basic functionality", "[utilities]") {
	compression_vector<int> v(std::vector<int>{2, 4, 3, 5, 8, 3, 5, 5});
	v.push_back(4);
	REQUIRE(v.size() == 9);
	v.compress();
	REQUIRE(v.size() == 5);
	REQUIRE(v[0] == 2);
	REQUIRE(v[2] == 4);
	REQUIRE(v[4] == 8);
	REQUIRE(v.lower_bound(10) == v.end());
	REQUIRE(v.lower_bound_index(3) == 1);
	REQUIRE(v.upper_bound(3) == v.begin() + 2);
	REQUIRE(v.upper_bound_index(0) == 0);
}

TEST_CASE("compression_vector projections", "[utilities]") {
	std::vector<int> a{2, 4, 3, 5, 8, 3, 5, 5, 4};
	std::vector<int> p{1, 0, 2, 4, 3, 5, 6, 7, 8};
	auto a_indexer = [&](int idx) { return a[idx]; };
	SECTION("projection works correctly") {
		compression_vector<int, std::ranges::less, std::ranges::equal_to,
		                   decltype(a_indexer)>
		    v(p, {}, {}, a_indexer);
		v.compress();
		REQUIRE(v.size() == 5);
		REQUIRE(v[4] == 4);
	}
	SECTION("CTAD works correctly") {
		compression_vector v(p, std::ranges::less{}, std::ranges::equal_to{},
		                     a_indexer);
		v.compress();
		REQUIRE(v.size() == 5);
		REQUIRE(v[4] == 4);
		compression_vector v2(p.begin(), p.end(), std::ranges::less{},
		                      std::ranges::equal_to{}, a_indexer);
		v2.compress();
		REQUIRE(v2.size() == 5);
		REQUIRE(v2[4] == 4);
	}
}

TEST_CASE("compression_vector constructors", "[utilities]") {
	compression_vector<std::string> v1(3, "hello");
	v1.compress();
	REQUIRE(v1.size() == 1);
	compression_vector<std::string> v2;
	v2.compress();
	REQUIRE(v2.empty());
	compression_vector<std::string> v3(4);
	v3.compress();
	REQUIRE(v3[0] == "");
}
