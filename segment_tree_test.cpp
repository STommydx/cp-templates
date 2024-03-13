#include "segment_tree.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

#include "functional.hpp"
#include "modint.hpp"

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

TEST_CASE(
    "segment_tree: range min query, point assignment update, with projection",
    "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	std::vector<int> p(v.size());
	std::iota(p.begin(), p.end(), 0);
	fn::minimum min_obj({}, [&](int idx) { return v[idx]; });
	segment_tree st(p, min_obj, fn::assign{});
	REQUIRE(st.query(0, 1) == 1);
	v[0] = 0;
	st.modify(0, 0);
	REQUIRE(st.query(0, 1) == 0);
	v[2] = 4;
	st.modify(2, 2);
	REQUIRE(st.query(2, 3) == 2);
	REQUIRE(st.query(1, 3) == 1);
}

TEST_CASE("segment_tree check order preserving: range rightmost query, point "
          "assignment update",
          "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	segment_tree<int, int, fn::assign, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 2);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 10);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 5);
	REQUIRE(st.query(1, 2) == 4);
}

TEST_CASE("segment_tree check order preserving: range leftmost query, point "
          "assignment update",
          "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	segment_tree<int, int, fn::noop, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 3);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 10);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 4);
	REQUIRE(st.query(1, 2) == 10);
}

TEST_CASE("lazy_segment_tree: range max query, range add update",
          "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	lazy_segment_tree<int, int, fn::maximum<>> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 18);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 1) == 12);
}

TEST_CASE("lazy_segment_tree: range max query, range assign update",
          "[segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	lazy_segment_tree<int, int, fn::maximum<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 10);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 1) == 10);
}

TEST_CASE("lazy_segment_tree: range gcd query, range assign update",
          "[segment_tree]") {
	std::vector<int> v{5, 2, 8, 6};
	lazy_segment_tree<int, int, fn::gcd<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 1);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(2, 3) == 2);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(0, 0) == 5);
	REQUIRE(st.query(0, 1) == 5);
}

TEST_CASE("lazy_segment_tree: range gcd query, range product update",
          "[segment_tree]") {
	std::vector<int> v{5, 2, 8, 6};
	lazy_segment_tree<int, int, fn::gcd<>, std::multiplies<>> st(v);
	REQUIRE(st.query(0, 1) == 1);
	st.modify(1, 2, 5);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(0, 0) == 5);
	REQUIRE(st.query(0, 1) == 5);
	REQUIRE(st.query(1, 2) == 10);
}

TEST_CASE("lazy_segment_tree: range product query, range product update",
          "[segment_tree]") {
	using mint = modint<int, 13>;
	std::vector<mint> v{1, 3, 4, 7, 2};
	lazy_segment_tree<mint, mint, std::multiplies<>, std::multiplies<>,
	                  std::multiplies<>, std::bit_xor<>>
	    st(v);
	REQUIRE(st.query(0, 2) == 12);
	REQUIRE(st.query(1, 3) == 6);
	st.modify(0, 2, 2);
	REQUIRE(st.query(0, 1) == 12);
	REQUIRE(st.query(1, 3) == 11);
	st.modify(1, 3, 5);
	REQUIRE(st.query(0, 0) == 2);
	REQUIRE(st.query(1, 3) == 10);
}

TEST_CASE("lazy_segment_tree: range mxss query, range assignment update",
          "[segment_tree]") {
	struct mxss_node {
		int l_ans, r_ans, ans, sum;
		mxss_node() = default;
		mxss_node(int v) : l_ans(v), r_ans(v), ans(v), sum(v) {}
		mxss_node(int l_ans, int r_ans, int ans, int sum)
		    : l_ans(l_ans), r_ans(r_ans), ans(ans), sum(sum) {}
		mxss_node operator+(mxss_node rhs) {
			return mxss_node{
			    std::max(l_ans, sum + rhs.l_ans),
			    std::max(r_ans + rhs.sum, rhs.r_ans),
			    std::max(std::max(ans, rhs.ans), r_ans + rhs.l_ans),
			    sum + rhs.sum};
		}
	};
	std::vector<int> v{-2, 1, -3, 4, -1, 2, 1, -5, 4};
	std::vector<mxss_node> v2(v.begin(), v.end());
	lazy_segment_tree<mxss_node, int, std::plus<>, fn::assign, fn::assign,
	                  decltype([](int x, int len) {
		                  return mxss_node{std::max(x, x * len),
		                                   std::max(x, x * len),
		                                   std::max(x, x * len), x * len};
	                  })>
	    st(v2);
	REQUIRE(st.query(1, 2).ans == 1);
	REQUIRE(st.query(0, 2).sum == -4);
	REQUIRE(st.query(0, 3).l_ans == 0);
	REQUIRE(st.query(1, 3).r_ans == 4);
	REQUIRE(st.query(7, 7).ans == -5);
	REQUIRE(st.query(3, 6).ans == 6);
	REQUIRE(st.query(0, 8).ans == 6);
	st.modify(6, 7, -1);
	REQUIRE(st.query(0, 8).ans == 7);
	REQUIRE(st.query(6, 7).ans == -1);
	REQUIRE(st.query(6, 8).sum == 2);
	REQUIRE(st.query(7, 8).r_ans == 4);
	st.modify(1, 2, 0);
	REQUIRE(st.query(0, 8).ans == 7);
	REQUIRE(st.query(0, 2).l_ans == -2);
	REQUIRE(st.query(1, 2).r_ans == 0);
}
