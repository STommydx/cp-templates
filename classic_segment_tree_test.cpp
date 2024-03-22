#include "classic_segment_tree.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

#include "functional.hpp"
#include "modint.hpp"

TEST_CASE("classic_segment_tree: range sum query, point add update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_segment_tree st(v);
	REQUIRE(st.query(0, 1) == 5);
	REQUIRE(st.query(1, 3) == 15);
	REQUIRE(st.query(2, 3) == 13);
	st.modify(1, 10);
	REQUIRE(st.query(1, 3) == 25);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 3) == 28);
}

TEST_CASE("classic_segment_tree: range max query, point add update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_segment_tree<int, int, fn::maximum<>> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 10);
	REQUIRE(st.query(1, 3) == 12);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 3) == 12);
}

TEST_CASE("classic_segment_tree: range min query, point min update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_segment_tree<int, int, fn::minimum<>, fn::minimum<>> st(v);
	REQUIRE(st.query(0, 1) == 2);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 2);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 4);
}

TEST_CASE("classic_segment_tree: range min query, point assignment update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_segment_tree<int, int, fn::minimum<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 2);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 10);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 4);
}

TEST_CASE("classic_segment_tree: range min query, point assignment update, "
          "with projection",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	std::vector<int> p(v.size());
	std::iota(p.begin(), p.end(), 0);
	fn::minimum min_obj({}, [&](int idx) { return v[idx]; });
	classic_segment_tree st(p, min_obj, fn::assign{});
	REQUIRE(st.query(0, 1) == 1);
	v[0] = 0;
	st.modify(0, 0);
	REQUIRE(st.query(0, 1) == 0);
	v[2] = 4;
	st.modify(2, 2);
	REQUIRE(st.query(2, 3) == 2);
	REQUIRE(st.query(1, 3) == 1);
}

TEST_CASE("classic_segment_tree check order preserving: range rightmost query, "
          "point assignment update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_segment_tree<int, int, fn::assign, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 2);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 10);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 5);
	REQUIRE(st.query(1, 2) == 4);
}

TEST_CASE("classic_segment_tree check order preserving: range leftmost query, "
          "point assignment update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_segment_tree<int, int, fn::noop, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 3);
	st.modify(1, 10);
	REQUIRE(st.query(1, 1) == 10);
	st.modify(2, 4);
	REQUIRE(st.query(2, 3) == 4);
	REQUIRE(st.query(1, 2) == 10);
}

TEST_CASE("classic_lazy_segment_tree: range max query, range add update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_lazy_segment_tree<int, int, fn::maximum<>> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 18);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 1) == 12);
}

TEST_CASE("classic_lazy_segment_tree: range max query, range assign update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	classic_lazy_segment_tree<int, int, fn::maximum<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 10);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 1) == 10);
}

TEST_CASE("classic_lazy_segment_tree: range gcd query, range assign update",
          "[classic_segment_tree]") {
	std::vector<int> v{5, 2, 8, 6};
	classic_lazy_segment_tree<int, int, fn::gcd<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 1);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(2, 3) == 2);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(0, 0) == 5);
	REQUIRE(st.query(0, 1) == 5);
}

TEST_CASE("classic_lazy_segment_tree: range gcd query, range product update",
          "[classic_segment_tree]") {
	std::vector<int> v{5, 2, 8, 6};
	classic_lazy_segment_tree<int, int, fn::gcd<>, std::multiplies<>> st(v);
	REQUIRE(st.query(0, 1) == 1);
	st.modify(1, 2, 5);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(0, 0) == 5);
	REQUIRE(st.query(0, 1) == 5);
	REQUIRE(st.query(1, 2) == 10);
}

TEST_CASE(
    "classic_lazy_segment_tree: range product query, range product update",
    "[classic_segment_tree]") {
	using mint = modint<int, 13>;
	std::vector<mint> v{1, 3, 4, 7, 2};
	classic_lazy_segment_tree<mint, mint, std::multiplies<>, std::multiplies<>,
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

TEST_CASE(
    "classic_lazy_segment_tree: range mxss query, range assignment update",
    "[classic_segment_tree]") {
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
	classic_lazy_segment_tree<mxss_node, int, std::plus<>, fn::assign,
	                          fn::assign, decltype([](int x, int len) {
		                          return mxss_node{std::max(x, x * len),
		                                           std::max(x, x * len),
		                                           std::max(x, x * len),
		                                           x * len};
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

TEST_CASE("dynamic_segment_tree: range max query, range add update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	dynamic_segment_tree<int, int, fn::maximum<>> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 18);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 1) == 12);
}

TEST_CASE("dynamic_segment_tree: range max query, range assign update",
          "[classic_segment_tree]") {
	std::vector<int> v{3, 2, 8, 5};
	dynamic_segment_tree<int, int, fn::maximum<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 3);
	REQUIRE(st.query(1, 3) == 8);
	REQUIRE(st.query(2, 3) == 8);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 10);
	REQUIRE(st.query(0, 0) == 3);
	REQUIRE(st.query(0, 1) == 10);
}

TEST_CASE("dynamic_segment_tree: range gcd query, range assign update",
          "[classic_segment_tree]") {
	std::vector<int> v{5, 2, 8, 6};
	dynamic_segment_tree<int, int, fn::gcd<>, fn::assign> st(v);
	REQUIRE(st.query(0, 1) == 1);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(2, 3) == 2);
	st.modify(1, 2, 10);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(0, 0) == 5);
	REQUIRE(st.query(0, 1) == 5);
}

TEST_CASE("dynamic_segment_tree: range gcd query, range product update",
          "[classic_segment_tree]") {
	std::vector<int> v{5, 2, 8, 6};
	dynamic_segment_tree<int, int, fn::gcd<>, std::multiplies<>> st(v);
	REQUIRE(st.query(0, 1) == 1);
	st.modify(1, 2, 5);
	REQUIRE(st.query(1, 3) == 2);
	REQUIRE(st.query(0, 0) == 5);
	REQUIRE(st.query(0, 1) == 5);
	REQUIRE(st.query(1, 2) == 10);
}

TEST_CASE("dynamic_segment_tree: range product query, range product update",
          "[classic_segment_tree]") {
	using mint = modint<int, 13>;
	std::vector<mint> v{1, 3, 4, 7, 2};
	dynamic_segment_tree<mint, mint, std::multiplies<>, std::multiplies<>,
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

TEST_CASE("dynamic_segment_tree: range mxss query, range assignment update",
          "[classic_segment_tree]") {
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
	dynamic_segment_tree<mxss_node, int, std::plus<>, fn::assign, fn::assign,
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

TEST_CASE(
    "dynamic_segment_tree: range max query, range add update, large indices",
    "[classic_segment_tree]") {
	dynamic_segment_tree<int, int, fn::maximum<>> st(1'000'000'000, 3);
	REQUIRE(st.query(0, 5) == 3);
	REQUIRE(st.query(238, 415) == 3);
	REQUIRE(st.query(998'244'353, 999'999'999) == 3);
	st.modify(1'234, 998'244'998, 10);
	st.modify(897, 1'432, 5);
	REQUIRE(st.query(1'231, 1'235) == 18);
	REQUIRE(st.query(0, 899) == 8);
	REQUIRE(st.query(23'456, 23'488) == 13);
}

TEST_CASE(
    "dynamic_segment_tree: range max query, range assign update, large indices",
    "[classic_segment_tree]") {
	dynamic_segment_tree<int, int, fn::maximum<>, fn::assign> st(1'000'000'000,
	                                                             3);
	REQUIRE(st.query(0, 5) == 3);
	REQUIRE(st.query(238, 415) == 3);
	REQUIRE(st.query(998'244'353, 999'999'999) == 3);
	st.modify(1'234, 998'244'998, 10);
	st.modify(897, 1'432, 5);
	REQUIRE(st.query(1'231, 1'235) == 5);
	REQUIRE(st.query(0, 899) == 5);
	REQUIRE(st.query(23'456, 23'488) == 10);
}

TEST_CASE(
    "dynamic_segment_tree: range gcd query, range assign update, large indices",
    "[classic_segment_tree]") {
	dynamic_segment_tree<int, int, fn::gcd<>, fn::assign> st(1'000'000'000, 18);
	st.modify(10, 13, 8);
	st.modify(13, 16, 6);
	st.modify(2, 11, 27);
	st.modify(998'244'353, 999'999'999, 5);
	REQUIRE(st.query(9, 12) == 1);
	REQUIRE(st.query(12, 15) == 2);
	REQUIRE(st.query(0, 3) == 9);
	REQUIRE(st.query(12, 2'048) == 2);
}

TEST_CASE(
    "dynamic_segment_tree: range sum query, range add update, large indices",
    "[classic_segment_tree]") {
	dynamic_segment_tree<long long, long long, std::plus<>, std::plus<>,
	                     std::plus<>, std::multiplies<>, long long>
	    st(6'000'000'000LL);
	st.modify(0LL, 4'000'000'000LL, 3);
	REQUIRE(st.query(1'000'000'000LL, 4'000'000'000LL) == 9'000'000'003LL);
	st.modify(3'000'000'000LL, 5'000'000'000LL, 7);
	REQUIRE(st.query(1'000'000'000LL, 4'000'000'000LL) == 16'000'000'010LL);
	dynamic_segment_tree<long long, long long, std::plus<>, std::plus<>,
	                     std::plus<>, std::multiplies<>, long long>
	    st2(6'000'000'000LL,
	        [](long long lo, long long hi) { return 3 * (hi - lo + 1); });
	REQUIRE(st2.query(1'000'000'000LL, 4'000'000'000LL) == 9'000'000'003LL);
}
