#include "mo.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <set>
#include <vector>

TEST_CASE("mo behaves as expected", "[mo]") {
	SECTION("queries are answered correctly") {
		mo mo(5);
		std::vector<int> v{0, 1, 1, 0, 2};
		std::multiset<int> st;
		mo.push_query(2, 3);
		mo.push_query(0, 2);
		mo.push_query(1, 3);
		mo.push_query(1, 2);
		auto result =
		    mo.solve<int>([&](int l, int r, size_t q_i) { return *st.begin(); },
		                  [&](int i) { st.insert(v[i]); },
		                  [&](int i) { st.erase(st.find(v[i])); },
		                  [&](int i) { st.insert(v[i]); },
		                  [&](int i) { st.erase(st.find(v[i])); });
		REQUIRE(result == std::vector<int>{0, 0, 0, 1});
	}
	BENCHMARK("n = 200000 should run reasonably fast") {
		const int n = 200'000;
		mo mo(n);
		std::vector<int> v(n);
		int seed = 0;
		for (int i = 0; i < n; i++) {
			v[i] = seed;
			seed = (seed * 37 + 13) % 1'007;
		}
		std::set<int> st;
		return mo.solve<int>(
		    [&](int l, int r, size_t q_i) { return *st.begin(); },
		    [&](int i) { st.insert(v[i]); },
		    [&](int i) { st.erase(st.find(v[i])); },
		    [&](int i) { st.insert(v[i]); },
		    [&](int i) { st.erase(st.find(v[i])); });
	};
}

TEST_CASE("mo with rollback", "[mo]") {
	SECTION("queries are answered correctly") {
		mo mo(10);
		std::vector<int> v{0, 1, 1, 0, 2, 3, 3, 2, 4, 5};
		mo.push_query(2, 3);
		mo.push_query(0, 2);
		mo.push_query(1, 4);
		mo.push_query(4, 5);
		mo.push_query(0, 0);
		mo.push_query(4, 8);
		mo.push_query(7, 9);
		int max_value = 0;
		std::vector<int> saves;
		auto result = mo.solve_rollback<int>(
		    [&](int l, int r, size_t q_i) { return max_value; },
		    [&]() { saves.push_back(max_value); },
		    [&]() {
			    max_value = saves.back();
			    saves.pop_back();
		    },
		    [&](int i) { max_value = std::max(max_value, v[i]); },
		    [&](int i) { max_value = std::max(max_value, v[i]); });
		REQUIRE(result == std::vector<int>{1, 1, 2, 3, 0, 4, 5});
	}
	BENCHMARK("n = 200000 should run reasonably fast") {
		const int n = 200'000;
		mo mo(n);
		std::vector<int> v(n);
		int seed = 0;
		for (int i = 0; i < n; i++) {
			v[i] = seed;
			seed = (seed * 37 + 13) % 1'007;
		}
		int max_value = 0;
		std::vector<int> saves;
		return mo.solve_rollback<int>(
		    [&](int l, int r, size_t q_i) { return max_value; },
		    [&]() { saves.push_back(max_value); },
		    [&]() {
			    max_value = saves.back();
			    saves.pop_back();
		    },
		    [&](int i) { max_value = std::max(max_value, v[i]); },
		    [&](int i) { max_value = std::max(max_value, v[i]); });
	};
}
