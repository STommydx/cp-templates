#include "cht.hpp"

#include <catch2/catch_test_macros.hpp>

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
