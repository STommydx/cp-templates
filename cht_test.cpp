#include "cht.hpp"

#include <catch2/catch_test_macros.hpp>
#include <functional>

#include <algorithm>
#include <vector>

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
	std::vector<cht<int>::line> lines{{4, 0}, {2, -5}, {0, -8}, {-2, -11}};
	cht<int> ascending(lines);
	cht<int, std::greater<>> descending(lines);
	for (int x = -20; x <= 20; x++) {
		int expected = lines[0].first * x + lines[0].second;
		for (auto [a, b] : lines)
			expected = std::min(expected, a * x + b);
		auto [ascending_value, ascending_index] = ascending.query(x);
		auto [descending_value, descending_index] = descending.query(x);
		REQUIRE(ascending_value == expected);
		REQUIRE(descending_value == expected);
		REQUIRE(lines[ascending_index].first * x +
		            lines[ascending_index].second ==
		        expected);
		REQUIRE(lines[descending_index].first * x +
		            lines[descending_index].second ==
		        expected);
	}
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
