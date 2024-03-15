#include "math.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("isqrt behaves as expected", "[math]") {
	REQUIRE(isqrt(0) == 0);
	REQUIRE(isqrt(1) == 1);
	REQUIRE(isqrt(2) == 1);
	REQUIRE(isqrt(3) == 1);
	REQUIRE(isqrt(4) == 2);
	REQUIRE(isqrt(5) == 2);
	REQUIRE(isqrt(8) == 2);
	REQUIRE(isqrt(9) == 3);
	REQUIRE(isqrt(10) == 3);
	REQUIRE(isqrt(12) == 3);
	REQUIRE(isqrt(15) == 3);
	REQUIRE(isqrt(16) == 4);
	REQUIRE(isqrt(1'000'000'000'000LL) == 1'000'000);
	REQUIRE(isqrt(1'000'000'000'000LL - 1) == 1'000'000 - 1);
}
