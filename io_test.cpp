#include "io.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

TEST_CASE("IO operator overload works for common cp types", "[io]") {
	SECTION("vector outputs elements separated by space") {
		std::stringstream ss;
		std::vector<std::string> v{"1", "22", "333"};
		ss << v;
		REQUIRE(ss.str() == "1 22 333");
	}
	SECTION("vector inputs elements separated by space") {
		std::stringstream ss{"1 22 333"};
		std::vector<std::string> v(3);
		ss >> v;
		REQUIRE(v == std::vector<std::string>({"1", "22", "333"}));
	}
	SECTION("2D vector outputs elements separated by space and newline") {
		std::stringstream ss;
		std::vector<std::vector<std::string>> v{{"1", "22", "333"},
		                                        {"4444", "55555", "666666"}};
		ss << v;
		REQUIRE(ss.str() == "1 22 333\n4444 55555 666666");
	}
	SECTION("2D vector inputs elements separated by space and newline") {
		std::stringstream ss{"1 22 333\n4444 55555 666666"};
		std::vector<std::vector<std::string>> v(2, std::vector<std::string>(3));
		ss >> v;
		REQUIRE(v == std::vector<std::vector<std::string>>(
		                 {{"1", "22", "333"}, {"4444", "55555", "666666"}}));
	}
	SECTION("valarray outputs elements separated by space") {
		std::stringstream ss;
		std::valarray<std::string> v{"1", "22", "333"};
		ss << v;
		REQUIRE(ss.str() == "1 22 333");
	}
	SECTION("valarray inputs elements separated by space") {
		std::stringstream ss{"1 22 333"};
		std::valarray<std::string> v(3);
		ss >> v;
		std::valarray<std::string> expected{"1", "22", "333"};
		REQUIRE((v == expected).min());
	}
	SECTION("2d valarray outputs elements separated by space and newline") {
		std::stringstream ss;
		std::valarray<std::valarray<std::string>> v{
		    {"1", "22", "333"}, {"4444", "55555", "666666"}};
		ss << v;
		REQUIRE(ss.str() == "1 22 333\n4444 55555 666666");
	}
	SECTION("2d valarray inputs elements separated by space and newline") {
		std::stringstream ss{"1 22 333\n4444 55555 666666"};
		std::valarray<std::valarray<std::string>> v(
		    std::valarray<std::string>(3), 2);
		ss >> v;
		REQUIRE(v[0][0] == "1");
		REQUIRE(v[1][1] == "55555");
		REQUIRE(v[1][2] == "666666");
	}
	SECTION("pair outputs elements separated by space") {
		std::stringstream ss;
		std::pair<std::string, int> p{"1", 22};
		ss << p;
		REQUIRE(ss.str() == "1 22");
	}
	SECTION("pair inputs elements separated by space") {
		std::stringstream ss{"1 22"};
		std::pair<std::string, int> p;
		ss >> p;
		REQUIRE(p == std::pair<std::string, int>({"1", 22}));
	}
	SECTION("tuple outputs elements separated by space") {
		std::stringstream ss;
		std::tuple<std::string, int, double> t{"1", 22, 3.3};
		ss << t;
		REQUIRE(ss.str() == "1 22 3.3");
	}
	SECTION("tuple inputs elements separated by space") {
		std::stringstream ss{"1 22 3.3"};
		std::tuple<std::string, int, double> t;
		ss >> t;
		REQUIRE(std::get<0>(t) == "1");
		REQUIRE(std::get<1>(t) == 22);
		REQUIRE(std::get<2>(t) == 3.3);
	}
	SECTION("vector of pairs outputs elements separated by space and newline") {
		std::stringstream ss;
		std::vector<std::pair<std::string, int>> v{
		    {"1", 22}, {"4444", 55555}, {"666666", 7777777}};
		ss << v;
		REQUIRE(ss.str() == "1 22\n4444 55555\n666666 7777777");
	}
	SECTION(
	    "vector of tuples outputs elements separated by space and newline") {
		std::stringstream ss;
		std::vector<std::tuple<std::string, int, double>> v{
		    {"1", 22, 3.3}, {"4444", 55555, 6.6}, {"666666", 7777777, 8.8}};
		ss << v;
		REQUIRE(ss.str() == "1 22 3.3\n4444 55555 6.6\n666666 7777777 8.8");
	}
}

TEST_CASE("I/O for 1-based indices", "[io]") {
	SECTION("vector of 1-based indices") {
		std::stringstream ss;
		std::vector<int> p{3, 0, 2, 1};
		ss << put_indices(p);
		REQUIRE(ss.str() == "4 1 3 2");
		std::vector<int> p2(4);
		ss >> get_indices(p2);
		REQUIRE(p == p2);
	}
	SECTION("2D vector of 1-based indices") {
		std::stringstream ss;
		std::vector<std::vector<int>> v{{1, 2, 3, 4}, {5, 6, 7, 8}};
		ss << put_indices(v);
		REQUIRE(ss.str() == "2 3 4 5\n6 7 8 9");
		std::vector<std::vector<int>> v2(2, std::vector<int>(4));
		ss >> get_indices(v2);
		REQUIRE(v == v2);
	}
	SECTION("vector of pairs of 1-based indices") {
		std::stringstream ss;
		std::vector<std::pair<int, int>> p{{3, 0}, {0, 2}, {2, 1}};
		ss << put_indices(p);
		REQUIRE(ss.str() == "4 1\n1 3\n3 2");
		std::vector<std::pair<int, int>> p2(3);
		ss >> get_indices(p2);
		REQUIRE(p == p2);
	}
	SECTION("vector of vector of tuples of 1-based indices") {
		std::stringstream ss;
		std::vector<std::vector<std::tuple<int, int, int>>> v{
		    {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
		    {{10, 11, 12}, {13, 14, 15}, {16, 17, 18}}};
		ss << put_indices(v);
		REQUIRE(ss.str() ==
		        "2 3 4\n5 6 7\n8 9 10\n11 12 13\n14 15 16\n17 18 19");
		std::vector<std::vector<std::tuple<int, int, int>>> v2(
		    2, std::vector<std::tuple<int, int, int>>(3));
		ss >> get_indices(v2);
		REQUIRE(v == v2);
	}
	SECTION("valarray of 1-based indices") {
		std::stringstream ss;
		std::valarray<int> p{3, 0, 2, 1};
		ss << put_indices(p);
		REQUIRE(ss.str() == "4 1 3 2");
		std::valarray<int> p2(4);
		ss >> get_indices(p2);
		REQUIRE((p == p2).min());
	}
}
