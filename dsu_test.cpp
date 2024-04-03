#include "dsu.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dsu behaves as expected", "[dsu]") {
	dsu<> d(5);
	SECTION("dsu constructed as expected") {
		REQUIRE(d.find_set(0) == 0);
		REQUIRE(d.find_set(1) == 1);
		REQUIRE(d.find_set(4) == 4);
	}
	SECTION("dsu union behaves as expected") {
		REQUIRE(d.union_sets(0, 1) == 1); // same size, v is selected
		REQUIRE(d.union_sets(1, 2) == 1); // larger component is selected
		REQUIRE(d.union_sets(3, 4) == 4);
		REQUIRE(d.union_sets(0, 2) == dsu<>::same_set);
	}
	SECTION("dsu find_set behaves as expected") {
		d.union_sets(0, 1);
		d.union_sets(1, 2);
		d.union_sets(3, 4);
		REQUIRE(d.find_set(0) == 1);
		REQUIRE(d.find_set(2) == 1);
		REQUIRE(d.find_set(3) == 4);
	}
	SECTION("dsu size_of_set behaves as expected") {
		d.union_sets(0, 1);
		REQUIRE(d.size_of_set(1) == 2);
		d.union_sets(1, 2);
		d.union_sets(3, 4);
		REQUIRE(d.size_of_set(1) == 3);
		REQUIRE(d.size_of_set(3) == 2);
		REQUIRE(d.size_of_set(4) == 2);
	}
}

TEST_CASE("dsu with data", "[dsu]") {
	std::vector<int> v{1, 2, 3, 4, 5};
	dsu<int, std::multiplies<>> d(v);
	d.union_sets(0, 1);
	REQUIRE(d.size_of_set(1) == 2);
	REQUIRE(d[1] == 2);
	d.union_sets(1, 2);
	d.union_sets(3, 4);
	REQUIRE(d.size_of_set(1) == 3);
	REQUIRE(d[1] == 6);
	REQUIRE(d.size_of_set(3) == 2);
	REQUIRE(d[3] == 20);
	REQUIRE(d.size_of_set(4) == 2);
	REQUIRE(d[4] == 20);
}
