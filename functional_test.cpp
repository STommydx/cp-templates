#include "functional.hpp"

#include <catch2/catch_test_macros.hpp>

#include <utility>
#include <vector>

TEST_CASE("fn::minimum object", "[functional]") {
	SECTION("default construction") {
		fn::minimum min_obj;
		REQUIRE(min_obj(1, 2) == 1);
		REQUIRE(min_obj(2, 1) == 1);
		REQUIRE(min_obj(2, 2) == 2);
	}
	SECTION("construction with custom comparator") {
		using pii = std::pair<int, int>;
		fn::minimum min_obj([](pii lhs, pii rhs) {
			return lhs.first * rhs.second < rhs.first * lhs.second;
		});
		REQUIRE(min_obj(pii{3, 5}, pii{1, 2}) == pii{1, 2});
	}
	SECTION("construction with custom projection") {
		std::vector v{3, 2, 5, 7};
		fn::minimum min_obj({}, [&](int idx) { return v[idx]; });
		REQUIRE(min_obj(0, 1) == 1);
		REQUIRE(min_obj(2, 3) == 2);
	}
}

TEST_CASE("fn::maximum object", "[functional]") {
	SECTION("default construction") {
		fn::maximum max_obj;
		REQUIRE(max_obj(1, 2) == 2);
		REQUIRE(max_obj(2, 1) == 2);
		REQUIRE(max_obj(1, 1) == 1);
	}
	SECTION("construction with custom comparator") {
		using pii = std::pair<int, int>;
		fn::maximum max_obj([](pii lhs, pii rhs) {
			return lhs.first * rhs.second < rhs.first * lhs.second;
		});
		REQUIRE(max_obj(pii{3, 5}, pii{1, 2}) == pii{3, 5});
	}
	SECTION("construction with custom projection") {
		std::vector v{3, 2, 5, 7};
		fn::maximum max_obj({}, [&](int idx) { return v[idx]; });
		REQUIRE(max_obj(0, 1) == 0);
		REQUIRE(max_obj(2, 3) == 3);
	}
}

TEST_CASE("fn::gcd object", "[functional]") {
	SECTION("default construction") {
		fn::gcd gcd_obj;
		REQUIRE(gcd_obj(3, 5) == 1);
		REQUIRE(gcd_obj(12LL, 9) == 3);
		REQUIRE(gcd_obj(5, 0) == 5);
	}
	SECTION("typed construction") {
		fn::gcd<int> gcd_obj;
		REQUIRE(gcd_obj(3, 5) == 1);
		REQUIRE(gcd_obj(12, 9) == 3);
		REQUIRE(gcd_obj(5, 0) == 5);
	}
}
