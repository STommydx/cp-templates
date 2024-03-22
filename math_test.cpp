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

TEST_CASE("fraction_cmp behaves as expected", "[math]") {
	REQUIRE(fraction_cmp(2, 5, 4, 10) == 0);
	REQUIRE(fraction_cmp(2, 3, 3, 4) < 0);
	REQUIRE(fraction_cmp(22, 7, 314, 100) > 0);
	REQUIRE(fraction_cmp(-1, 2, 1, 2) > 0);
	REQUIRE(fraction_cmp(-1, 3, -1, 2) > 0);
	REQUIRE(fraction_cmp(1, -3, 0, -2) < 0);
	REQUIRE(fraction_cmp(-19, 17, -38, 34) == 0);
	REQUIRE(fraction_cmp(0, 1, 0, 1783) == 0);
	REQUIRE(fraction_cmp(9'999'999'998LL, 9'999'999'999LL, 9'999'999'997LL,
	                     9'999'999'998LL) > 0);
	REQUIRE(fraction_cmp(-9'999'999'998LL, 9'999'999'999LL, -9'999'999'997LL,
	                     9'999'999'998LL) < 0);
}
