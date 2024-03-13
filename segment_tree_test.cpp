#include "functional.hpp"
#include "segment_tree.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("segment_tree: range sum query, point add update", "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	segment_tree st(v);
	REQUIRE(st.query(0, 1) == 5);
	REQUIRE(st.query(1, 3) == 15);
	REQUIRE(st.query(2, 3) == 13);
	st.modify(1, 10);
	REQUIRE(st.query(1, 3) == 25);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 3) == 28);
}

TEST_CASE("segment_tree: range max query, point add update", "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	segment_tree<int, int, fn::maximum<>> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 10);
	REQUIRE(st.query(1, 3) == 12);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 3) == 12);
}

TEST_CASE("segment_tree: range min query, point min update", "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	segment_tree<int, int, fn::minimum<>, fn::minimum<>> st(v);
	REQUIRE(st.query(0, 1) == 2);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 2);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 4);
}

TEST_CASE("segment_tree: range min query, point assignment update",
          "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	segment_tree<int, int, fn::minimum<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 2);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 10);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 4);
}
