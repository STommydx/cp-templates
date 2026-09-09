#include "flow.hpp"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("dinic max flow", "[flow]") {
	flow_net<int> mxf(4, 3, 2);
	mxf.push_edge(3, 1, 30);
	mxf.push_edge(3, 2, 20);
	mxf.push_edge(1, 2, 20);
	mxf.push_edge(1, 0, 30);
	mxf.push_edge(0, 2, 30);
	REQUIRE(mxf.dinic_max_flow() == 50);
}

TEST_CASE("dinic ssp min cost max flow", "[flow]") {
	cost_flow_net<int> mcmf(4, 3, 2);
	mcmf.push_edge(3, 1, 30, 2);
	mcmf.push_edge(3, 2, 20, 3);
	mcmf.push_edge(1, 2, 20, 1);
	mcmf.push_edge(1, 0, 30, 9);
	mcmf.push_edge(0, 2, 30, 5);
	auto [cost, flow] = mcmf.dinic_mcmf();
	REQUIRE(flow == 50);
	REQUIRE(cost == 280);
}

TEST_CASE("primal dual min cost max flow", "[flow]") {
	cost_flow_net<int> mcmf(4, 3, 2);
	mcmf.push_edge(3, 1, 30, 2);
	mcmf.push_edge(3, 2, 20, 3);
	mcmf.push_edge(1, 2, 20, 1);
	mcmf.push_edge(1, 0, 30, 9);
	mcmf.push_edge(0, 2, 30, 5);
	auto [cost, flow] = mcmf.primal_dual_mcmf();
	REQUIRE(flow == 50);
	REQUIRE(cost == 280);
}

TEST_CASE("min-cost flow rejects reachable negative cycles", "[flow]") {
	auto make_network = [] {
		cost_flow_net<int> network(4, 0, 3);
		network.push_edge(0, 1, 1, 0);
		network.push_edge(1, 2, 1, -1);
		network.push_edge(2, 1, 1, 0);
		network.push_edge(2, 3, 1, 0);
		return network;
	};
	REQUIRE_THROWS_AS(make_network().dinic_mcmf(), std::runtime_error);
	REQUIRE_THROWS_AS(make_network().primal_dual_mcmf(), std::runtime_error);
}
