#include "graph.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

TEST_CASE("unweighted graph functionality test", "[graph]") {
	graph<> g(4);
	g.push_edge(0, 1);
	g.push_edge(1, 2);
	g.push_edge(2, 0);
	g.push_edge(2, 3);

	SECTION("unweighted graph contains all edges") {
		REQUIRE(g.size() == 4);
		REQUIRE(g[0].size() == 1);
		REQUIRE(g[1].size() == 1);
		REQUIRE(g[2].size() == 2);
		REQUIRE(g[3].size() == 0);
	}

	SECTION("unweighted graph has correct edges") {
		REQUIRE(g[0][0] == 1);
		REQUIRE(g[1][0] == 2);
		REQUIRE(g[2][0] == 0);
		REQUIRE(g[2][1] == 3);
	}

	SECTION("unweighted graph has correct in degree") {
		REQUIRE(g.get_in_degree() == std::vector<int>{1, 1, 1, 1});
	}

	SECTION("unweighted graph has correct out degree") {
		REQUIRE(g.get_out_degree() == std::vector<int>{1, 1, 2, 0});
	}

	SECTION("reversed unweighted graph is correct") {
		graph<> g_rev = g.reversed();
		REQUIRE(g_rev.size() == 4);
		REQUIRE(g.get_in_degree() == g_rev.get_out_degree());
	}

	SECTION("adjacency matrix is correct") {
		REQUIRE(g.adjacency_matrix() ==
		        std::vector<std::vector<bool>>{
		            {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}, {0, 0, 0, 0}});
	}

	SECTION("edge list is correct") {
		REQUIRE(g.edge_list() ==
		        std::vector<graph<>::edge>{
		            std::pair<int, int>(0, 1), std::pair<int, int>(1, 2),
		            std::pair<int, int>(2, 0), std::pair<int, int>(2, 3)});
	}
}

TEST_CASE("weighted graph functionality test", "[graph]") {
	graph<int> g(4);
	g.push_edge(0, 1, 4);
	g.push_edge(1, 2, 1);
	g.push_edge(2, 0, 5);
	g.push_edge(2, 3, 8);

	SECTION("weighted graph contains all edges") {
		REQUIRE(g.size() == 4);
		REQUIRE(g[0].size() == 1);
		REQUIRE(g[1].size() == 1);
		REQUIRE(g[2].size() == 2);
		REQUIRE(g[3].size() == 0);
	}

	SECTION("weighted graph has correct edges") {
		REQUIRE(g[0][0] == 1);
		REQUIRE(g[1][0] == 2);
		REQUIRE(g[2][0] == 0);
		REQUIRE(g[2][1] == 3);
		REQUIRE(g.dat[0][0] == 4);
		REQUIRE(g.dat[1][0] == 1);
		REQUIRE(g.dat[2][0] == 5);
		REQUIRE(g.dat[2][1] == 8);
	}

	SECTION("adjacency matrix is correct") {
		REQUIRE(g.adjacency_matrix() ==
		        std::vector<std::vector<int>>{
		            {0, 4, 0, 0}, {0, 0, 1, 0}, {5, 0, 0, 8}, {0, 0, 0, 0}});
	}

	SECTION("edge list is correct") {
		REQUIRE(g.edge_list() == std::vector<graph<int>::edge>{
		                             std::pair<std::pair<int, int>, int>(
		                                 std::pair<int, int>(0, 1), 4),
		                             std::pair<std::pair<int, int>, int>(
		                                 std::pair<int, int>(1, 2), 1),
		                             std::pair<std::pair<int, int>, int>(
		                                 std::pair<int, int>(2, 0), 5),
		                             std::pair<std::pair<int, int>, int>(
		                                 std::pair<int, int>(2, 3), 8)});
	}
}

TEST_CASE("DFS graph traversal", "[graph]") {
	graph<> g(4);
	g.push_edge(0, 1);
	g.push_edge(1, 2);
	g.push_edge(2, 0);
	g.push_edge(2, 3);
	auto [pre_order, post_order, depth, parent] = g.dfs_traversal();
	REQUIRE(pre_order == std::vector<int>{0, 1, 2, 3});
	REQUIRE(post_order == std::vector<int>{3, 2, 1, 0});
	REQUIRE(depth == std::vector<int>{0, 1, 2, 3});
	REQUIRE(parent == std::vector<int>{graph<>::no_parent, 0, 1, 2});
}

TEST_CASE("BFS graph traversal", "[graph]") {
	graph<> g(4);
	g.push_edge(0, 1);
	g.push_edge(0, 2);
	g.push_edge(1, 2);
	g.push_edge(2, 0);
	g.push_edge(2, 3);
	auto [order, distance, parent] = g.bfs_traversal();
	REQUIRE(order == std::vector<int>{0, 1, 2, 3});
	REQUIRE(distance == std::vector<int>{0, 1, 1, 2});
	REQUIRE(parent == std::vector<int>{graph<>::no_parent, 0, 0, 2});
}

TEST_CASE("topological sort on DAG", "[graph]") {
	graph<> g(4);
	g.push_edge(0, 1);
	g.push_edge(2, 3);
	g.push_edge(0, 2);
	g.push_edge(1, 3);
	std::optional<std::vector<int>> sort_result = g.topological_sort();
	REQUIRE(sort_result);
	REQUIRE(*sort_result == std::vector<int>{0, 2, 1, 3});
}

TEST_CASE("topological sort on graph with cycles", "[graph]") {
	graph<> g(4);
	g.push_edge(0, 1);
	g.push_edge(1, 2);
	g.push_edge(2, 3);
	g.push_edge(3, 1);
	std::optional<std::vector<int>> sort_result = g.topological_sort();
	REQUIRE(!sort_result);
}

TEST_CASE("IO for unweighted graph", "[graph]") {
	std::stringstream ss{"1 2\n2 3\n3 1\n3 4"};
	graph<> g(4);
	ss >> get_graph(g, 4);
	REQUIRE(g.get_in_degree() == std::vector<int>{1, 1, 1, 1});
	REQUIRE(g.get_out_degree() == std::vector<int>{1, 1, 2, 0});
	std::stringstream bi_ss{"1 2\n2 3\n3 1\n3 4"};
	graph<> bi_g(4);
	bi_ss >> get_undirected_graph(bi_g, 4);
	REQUIRE(bi_g.get_in_degree() == std::vector<int>{2, 2, 3, 1});
	REQUIRE(bi_g.get_out_degree() == std::vector<int>{2, 2, 3, 1});
}

TEST_CASE("IO for weighted graph", "[graph]") {
	std::stringstream ss{"1 2 4\n2 3 1\n3 1 5\n3 4 8"};
	graph<int> g(4);
	ss >> get_graph(g, 4);
	REQUIRE(g.get_in_degree() == std::vector<int>{1, 1, 1, 1});
	REQUIRE(g.get_out_degree() == std::vector<int>{1, 1, 2, 0});
	std::stringstream bi_ss{"1 2 4\n2 3 1\n3 1 5\n3 4 8"};
	graph<int> bi_g(4);
	bi_ss >> get_undirected_graph(bi_g, 4);
	REQUIRE(bi_g.get_in_degree() == std::vector<int>{2, 2, 3, 1});
	REQUIRE(bi_g.get_out_degree() == std::vector<int>{2, 2, 3, 1});
}

TEST_CASE("IO for tree", "[graph]") {
	std::stringstream ss{"1 2\n2 3\n3 4"};
	graph<> g(4);
	ss >> get_tree(g);
	REQUIRE(g.get_in_degree() == std::vector<int>{0, 1, 1, 1});
	REQUIRE(g.get_out_degree() == std::vector<int>{1, 1, 1, 0});
	std::stringstream bi_ss{"1 2 5\n2 3 1\n3 4 0"};
	graph<int> bi_g(4);
	bi_ss >> get_undirected_tree(bi_g);
	REQUIRE(bi_g.get_in_degree() == std::vector<int>{1, 2, 2, 1});
	REQUIRE(bi_g.get_out_degree() == std::vector<int>{1, 2, 2, 1});
}

TEST_CASE("building connected components", "[graph]") {
	std::vector<graph<>::edge> edges{{0, 1}, {1, 0}, {1, 2},
	                                 {2, 1}, {3, 4}, {4, 3}};
	graph<> g(5, edges);
	auto [color, components] = g.get_connected_components();
	REQUIRE(color == std::vector<int>{0, 0, 0, 1, 1});
	REQUIRE(components[0] == std::vector<int>{0, 1, 2});
	REQUIRE(components[1] == std::vector<int>{3, 4});
}

TEST_CASE("dijkstra shortest path", "[graph]") {
	graph<int> g(4);
	g.push_edge(0, 1, 2);
	g.push_edge(1, 2, 2);
	g.push_edge(1, 3, 1);
	g.push_edge(0, 2, 5);
	g.push_edge(2, 3, 3);
	g.push_edge(0, 3, 4);
	auto [dist, par] = g.dijkstra(0);
	REQUIRE(dist == std::vector<int>{0, 2, 4, 3});
	REQUIRE(par == std::vector<int>{graph<int>::no_parent, 0, 1, 1});
}

TEST_CASE("lowest common ancestor", "[graph]") {
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
