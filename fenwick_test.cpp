#include "fenwick.hpp"
#include "modint.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("fenwick behaves as expected", "[fenwick]") {
	SECTION("fenwick constructed as expected") {
		fenwick<int> f(5);
		std::vector<long long> g(4, 3);
		fenwick g_tree(g);
		REQUIRE(f.query(0, 4) == 0);
		REQUIRE(g_tree.query(0, 3) == 12);
	}
	SECTION("fenwick query behaves as expected") {
		std::vector<long long> g{3, 2, 4, 5};
		fenwick g_tree(g);
		REQUIRE(g_tree.query(0, 0) == 3);
		REQUIRE(g_tree.query(0, 1) == 5);
		REQUIRE(g_tree.query(1, 3) == 11);
	}
	SECTION("fenwick modify behaves as expected") {
		std::vector<long long> g{3, 2, 4, 5};
		fenwick g_tree(g);
		g_tree.modify(1, 5);
		REQUIRE(g_tree.query(0, 0) == 3);
		REQUIRE(g_tree.query(0, 1) == 10);
		REQUIRE(g_tree.query(1, 3) == 16);
	}
}

TEST_CASE("fenwick with custom datatype", "[fenwick]") {
	fenwick<mint_1097> f(5);
	std::vector<mint_1097> g(4, -3);
	fenwick g_tree(g);
	REQUIRE(f.query(0, 4) == 0);
	REQUIRE(g_tree.query(0, 3) == -12);
}
