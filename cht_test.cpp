#include "cht.hpp"

#include <catch2/catch_test_macros.hpp>
#include <functional>

TEST_CASE("cht behaves as expected", "[cht]") {
	cht<int> c({{2, 1}, {3, 2}, {1, -2}});
	REQUIRE(c.size() == 2);
	REQUIRE(c.query(-4).first == -10);
	REQUIRE(c.query(-3).second == 1);
	REQUIRE(c.query(-3).first == -7);
	REQUIRE(c.query(-2).first == -4);
	REQUIRE(c.query(0).first == -2);
	REQUIRE(c.query(2).first == 0);
	REQUIRE(c.query(2).second == 2);
	cht<int> c2({{1, 0}, {2, 2}, {-1, 0}});
	REQUIRE(c2.size() == 3);
	REQUIRE(c2.query(-3).first == -4);
	REQUIRE(c2.query(-2).first == -2);
	REQUIRE(c2.query(-1).first == -1);
	REQUIRE(c2.query(0).first == 0);
	REQUIRE(c2.query(1).first == -1);
}

TEST_CASE("cht with custom ordering", "[cht]") {
	// Hull with default ascending order (std::less)
	cht<int> c_asc({{1, 3}, {3, 1}, {2, 2}});

	// Hull with descending order (std::greater)
	cht<int, std::greater<>> c_desc({{1, 3}, {3, 1}, {2, 2}});

	// Both should produce valid convex hulls
	REQUIRE(c_asc.size() >= 1);
	REQUIRE(c_desc.size() >= 1);

	// Test some queries to ensure both work correctly
	// Note: The actual results might differ because of the different ordering
	// but both should be mathematically correct
	REQUIRE(c_asc.query(0).first == c_desc.query(0).first);
	REQUIRE(c_asc.query(1).first == c_desc.query(1).first);
}

TEST_CASE("lichao_tree behaves as expected", "[cht]") {
	lichao_tree<int> c(-100, 100);
	c.push_line({2, 1});
	c.push_line({3, 2});
	c.push_line({1, -2});
	REQUIRE(c.query(-4).first == -10);
	REQUIRE(c.query(-3).second == 1);
	REQUIRE(c.query(-3).first == -7);
	REQUIRE(c.query(-2).first == -4);
	REQUIRE(c.query(0).first == -2);
	REQUIRE(c.query(2).first == 0);
	REQUIRE(c.query(2).second == 2);
	lichao_tree<int> c2(-100, 100);
	c2.push_line({1, 0});
	c2.push_line({2, 2});
	c2.push_line({-1, 0});
	REQUIRE(c2.query(-3).first == -4);
	REQUIRE(c2.query(-2).first == -2);
	REQUIRE(c2.query(-1).first == -1);
	REQUIRE(c2.query(0).first == 0);
	REQUIRE(c2.query(1).first == -1);
}
