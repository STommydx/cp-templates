#include "limits.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("limit for int", "[limits]") {
	REQUIRE(cp_limits<int>::infinity() == 0x3f3f3f3f);
	REQUIRE(cp_limits<int>::negative_infinity() == -0x3f3f3f3f);
}

TEST_CASE("limit for long long", "[limits]") {
	REQUIRE(cp_limits<long long>::infinity() == 0x3f3f3f3f3f3f3f3fLL);
	REQUIRE(cp_limits<long long>::negative_infinity() == -0x3f3f3f3f3f3f3f3fLL);
}

TEST_CASE("limit for pair<int, int>", "[limits]") {
	REQUIRE(cp_limits<std::pair<int, int>>::infinity() ==
	        std::make_pair(0x3f3f3f3f, 0x3f3f3f3f));
	REQUIRE(cp_limits<std::pair<int, int>>::negative_infinity() ==
	        std::make_pair(-0x3f3f3f3f, -0x3f3f3f3f));
}

TEST_CASE("limit for char", "[limits]") {
	REQUIRE(cp_limits<char>::infinity() > 'z');
	REQUIRE(cp_limits<char>::negative_infinity() < ' ');
}

TEST_CASE("limit for size_t", "[limits]") {
	REQUIRE(cp_limits<size_t>::infinity() < std::numeric_limits<size_t>::max());
	REQUIRE(cp_limits<size_t>::negative_infinity() == 0);
}

TEST_CASE("limit for short", "[limits]") {
	REQUIRE(cp_limits<short>::infinity() > 10000);
	REQUIRE(cp_limits<short>::negative_infinity() < -10000);
}
