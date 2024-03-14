#include "string.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("prefix function behaves as expected", "[string]") {
	REQUIRE(prefix_function("baobaba") ==
	        std::vector<int>{0, 0, 0, 1, 2, 1, 2, 0});
	std::string s = "baobaba";
	REQUIRE(prefix_function(s) == std::vector<int>{0, 0, 0, 1, 2, 1, 2});
	std::vector<int> v = {2, 2, 8, 1, 5, 3, 2, 2, 8};
	REQUIRE(prefix_function(v) == std::vector<int>{0, 1, 0, 0, 0, 0, 1, 2, 3});
}

TEST_CASE("kmp behaves as expected", "[string]") {
	std::string s = "ababa";
	std::string p = "aba";
	REQUIRE(kmp(s, p) == std::vector<int>{0, 2});
}
