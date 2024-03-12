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
