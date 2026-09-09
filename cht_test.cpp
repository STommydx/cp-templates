#include "cht.hpp"

#include <catch2/catch_test_macros.hpp>
#include <functional>

#include <algorithm>
#include <stdexcept>
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

TEST_CASE("cht handles duplicate slopes", "[cht]") {
	std::vector<cht<int>::line> lines{{2, 7}, {2, 3}, {-1, 0}};
	cht<int> ascending(lines);
	cht<int, std::greater<>> descending(lines);
	for (int x = -10; x <= 10; x++) {
		int expected = std::min(2 * x + 3, -x);
		REQUIRE(ascending.query(x).first == expected);
		REQUIRE(descending.query(x).first == expected);
	}
	REQUIRE(ascending.query(-10).second == 1);
	REQUIRE(descending.query(-10).second == 1);

	cht<int> tied({{1, 5}, {1, 5}, {0, 100}});
	REQUIRE(tied.query(0).second == 0);
}

TEST_CASE("cht rejects empty queries", "[cht]") {
	REQUIRE_THROWS_AS(cht<int>{}.query(0), std::out_of_range);
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

TEST_CASE("lichao_tree validates its domain and ranges", "[cht]") {
	REQUIRE_THROWS_AS(lichao_tree<int>(0, 0), std::invalid_argument);
	REQUIRE_THROWS_AS(lichao_tree<int>(2, 1), std::invalid_argument);

	lichao_tree<int> tree(0, 10);
	REQUIRE_THROWS_AS(tree.query(0), std::out_of_range);
	REQUIRE_THROWS_AS(tree.query(10), std::out_of_range);

	const std::pair<int, int> line{1, 0};
	REQUIRE_THROWS_AS(tree.modify(3, 2, line), std::out_of_range);
	REQUIRE_THROWS_AS(tree.modify(-1, 2, line), std::out_of_range);
	REQUIRE_THROWS_AS(tree.modify(0, 10, line), std::out_of_range);

	tree.modify(2, 4, line);
	REQUIRE(tree.query(2).first == 2);
	REQUIRE(tree.query(4).first == 4);
}
