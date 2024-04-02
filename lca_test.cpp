#include "lca.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("lowest common ancestor", "[lca]") {
	graph<> g(7);
	g.push_edge(0, 1);
	g.push_edge(0, 2);
	g.push_edge(1, 3);
	g.push_edge(1, 4);
	g.push_edge(4, 5);
	g.push_edge(4, 6);
	SECTION("LCA without data") {
		lca<> l(g);
		REQUIRE(l.kth_ancestor(0, 1) == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(3, 1) == 1);
		REQUIRE(l.kth_ancestor(3, 3) == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 1) == 4);
		REQUIRE(l.kth_ancestor(5, 2) == 1);
		REQUIRE(l.kth_ancestor(5, 3) == 0);
		REQUIRE(l.kth_ancestor(5, 4) == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(6, 1) == 4);
		REQUIRE(l.kth_ancestor(5, 4) == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 15) == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 128) == lca<>::no_parent);
		REQUIRE(l(5, 6) == 4);
		REQUIRE(l(3, 4) == 1);
		REQUIRE(l(2, 6) == 0);
		REQUIRE(l(5, 0) == 0);
	}
	SECTION("LCA with vertex query") {
		std::vector<int> dat(7, 1);
		lca<int> l(dat, g);
		REQUIRE(!l.kth_ancestor(0, 1).first.has_value());
		REQUIRE(l.kth_ancestor(3, 1).first.value() == 2);
		REQUIRE(!l.kth_ancestor(3, 3).first.has_value());
		REQUIRE(l.kth_ancestor(5, 1).first.value() == 2);
		REQUIRE(l.kth_ancestor(5, 2).first.value() == 3);
		REQUIRE(l.kth_ancestor(5, 3).first.value() == 4);
		REQUIRE(!l.kth_ancestor(5, 4).first.has_value());
		REQUIRE(l.kth_ancestor(6, 1).first.value() == 2);
		REQUIRE(!l.kth_ancestor(5, 4).first.has_value());
		REQUIRE(!l.kth_ancestor(5, 15).first.has_value());
		REQUIRE(!l.kth_ancestor(5, 128).first.has_value());
		REQUIRE(l(5, 6).first.value() == 3);
		REQUIRE(l(3, 4).first.value() == 3);
		REQUIRE(l(2, 6).first.value() == 5);
		REQUIRE(l(5, 0).first.value() == 4);
		REQUIRE(l.kth_ancestor(0, 1).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(3, 1).second == 1);
		REQUIRE(l.kth_ancestor(3, 3).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 1).second == 4);
		REQUIRE(l.kth_ancestor(5, 2).second == 1);
		REQUIRE(l.kth_ancestor(5, 3).second == 0);
		REQUIRE(l.kth_ancestor(5, 4).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(6, 1).second == 4);
		REQUIRE(l.kth_ancestor(5, 4).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 15).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 128).second == lca<>::no_parent);
		REQUIRE(l(5, 6).second == 4);
		REQUIRE(l(3, 4).second == 1);
		REQUIRE(l(2, 6).second == 0);
		REQUIRE(l(5, 0).second == 0);
	}
	SECTION("LCA with vertex query") {
		std::vector<int> dat(7, 1);
		lca<int, std::plus<>, false> l(dat, g);
		REQUIRE(!l.kth_ancestor(0, 1).first.has_value());
		REQUIRE(l.kth_ancestor(3, 1).first.value() == 1);
		REQUIRE(!l.kth_ancestor(3, 3).first.has_value());
		REQUIRE(l.kth_ancestor(5, 1).first.value() == 1);
		REQUIRE(l.kth_ancestor(5, 2).first.value() == 2);
		REQUIRE(l.kth_ancestor(5, 3).first.value() == 3);
		REQUIRE(!l.kth_ancestor(5, 4).first.has_value());
		REQUIRE(l.kth_ancestor(6, 1).first.value() == 1);
		REQUIRE(!l.kth_ancestor(5, 4).first.has_value());
		REQUIRE(!l.kth_ancestor(5, 15).first.has_value());
		REQUIRE(!l.kth_ancestor(5, 128).first.has_value());
		REQUIRE(l(5, 6).first.value() == 2);
		REQUIRE(l(3, 4).first.value() == 2);
		REQUIRE(l(2, 6).first.value() == 4);
		REQUIRE(l(5, 0).first.value() == 3);
		REQUIRE(l.kth_ancestor(0, 1).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(3, 1).second == 1);
		REQUIRE(l.kth_ancestor(3, 3).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 1).second == 4);
		REQUIRE(l.kth_ancestor(5, 2).second == 1);
		REQUIRE(l.kth_ancestor(5, 3).second == 0);
		REQUIRE(l.kth_ancestor(5, 4).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(6, 1).second == 4);
		REQUIRE(l.kth_ancestor(5, 4).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 15).second == lca<>::no_parent);
		REQUIRE(l.kth_ancestor(5, 128).second == lca<>::no_parent);
		REQUIRE(l(5, 6).second == 4);
		REQUIRE(l(3, 4).second == 1);
		REQUIRE(l(2, 6).second == 0);
		REQUIRE(l(5, 0).second == 0);
	}
}
