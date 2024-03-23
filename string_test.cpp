#include "string.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("prefix function behaves as expected", "[string]") {
	REQUIRE(prefix_function("baobaba") ==
	        std::vector<int>{0, 0, 0, 1, 2, 1, 2, 0});
	std::string s = "baobaba";
	REQUIRE(prefix_function(s) == std::vector<int>{0, 0, 0, 1, 2, 1, 2});
	std::vector<int> v = {2, 2, 8, 1, 5, 3, 2, 2, 8};
	REQUIRE(prefix_function(v) == std::vector<int>{0, 1, 0, 0, 0, 0, 1, 2, 3});
}

TEST_CASE("kmp behaves as expected", "[string]") {
	std::string s = "ababa";
	std::string p = "aba";
	REQUIRE(kmp(s, p) == std::vector<int>{0, 2});
}

TEST_CASE("charset conversions behave as expected", "[string]") {
	SECTION("lower") {
		REQUIRE(charset::lower{}.to_index('a') == 0);
		REQUIRE(charset::lower{}.to_char(0) == 'a');
		REQUIRE(charset::lower{}.to_index('j') == 9);
		REQUIRE(charset::lower{}.to_char(25) == 'z');
	}
	SECTION("upper") {
		REQUIRE(charset::upper{}.to_index('A') == 0);
		REQUIRE(charset::upper{}.to_char(0) == 'A');
		REQUIRE(charset::upper{}.to_index('J') == 9);
		REQUIRE(charset::upper{}.to_char(25) == 'Z');
	}
	SECTION("digit") {
		REQUIRE(charset::digit{}.to_index('0') == 0);
		REQUIRE(charset::digit{}.to_char(0) == '0');
		REQUIRE(charset::digit{}.to_index('1') == 1);
		REQUIRE(charset::digit{}.to_char(1) == '1');
		REQUIRE(charset::digit{}.to_index('9') == 9);
		REQUIRE(charset::digit{}.to_char(9) == '9');
	}
	SECTION("alpha") {
		REQUIRE(charset::alpha{}.to_index('A') == 0);
		REQUIRE(charset::alpha{}.to_char(0) == 'A');
		REQUIRE(charset::alpha{}.to_index('B') == 1);
		REQUIRE(charset::alpha{}.to_char(1) == 'B');
		REQUIRE(charset::alpha{}.to_index('Z') == 25);
		REQUIRE(charset::alpha{}.to_char(25) == 'Z');
		REQUIRE(charset::alpha{}.to_index('a') == 26);
		REQUIRE(charset::alpha{}.to_char(26) == 'a');
		REQUIRE(charset::alpha{}.to_index('b') == 27);
		REQUIRE(charset::alpha{}.to_char(27) == 'b');
		REQUIRE(charset::alpha{}.to_index('z') == 51);
		REQUIRE(charset::alpha{}.to_char(51) == 'z');
	}
	SECTION("alnum") {
		REQUIRE(charset::alnum{}.to_index('0') == 0);
		REQUIRE(charset::alnum{}.to_char(0) == '0');
		REQUIRE(charset::alnum{}.to_index('1') == 1);
		REQUIRE(charset::alnum{}.to_char(1) == '1');
		REQUIRE(charset::alnum{}.to_index('9') == 9);
		REQUIRE(charset::alnum{}.to_char(9) == '9');
		REQUIRE(charset::alnum{}.to_index('A') == 10);
		REQUIRE(charset::alnum{}.to_char(10) == 'A');
		REQUIRE(charset::alnum{}.to_index('B') == 11);
		REQUIRE(charset::alnum{}.to_char(11) == 'B');
		REQUIRE(charset::alnum{}.to_index('Z') == 35);
		REQUIRE(charset::alnum{}.to_char(35) == 'Z');
		REQUIRE(charset::alnum{}.to_index('a') == 36);
		REQUIRE(charset::alnum{}.to_char(36) == 'a');
		REQUIRE(charset::alnum{}.to_index('b') == 37);
		REQUIRE(charset::alnum{}.to_char(37) == 'b');
		REQUIRE(charset::alnum{}.to_index('z') == 61);
		REQUIRE(charset::alnum{}.to_char(61) == 'z');
	}
}

TEST_CASE("trie behaves as expected", "[string]") {
	trie t;
	t.insert("ababa");
	t.insert("ababa");
	t.insert("aba");
	t.insert("acba");
	REQUIRE(t.count("ababa") == 2);
	REQUIRE(t.count("ab") == 0);
	REQUIRE(t.count("ababaa") == 0);
	REQUIRE(t.count_prefix("aba") == 3);
	REQUIRE(t.count_prefix("ac") == 1);
	REQUIRE(t.count_prefix("") == 4);
	REQUIRE(t.count_prefix("c") == 0);
	REQUIRE(t.order_of_key("ababa") == 1);
	REQUIRE(t.order_of_key("acba") == 3);
	REQUIRE(t.order_of_key("zzz") == 4);
	REQUIRE(t.order_of_key("") == 0);
	REQUIRE(t.find_by_order(0) == "aba");
	REQUIRE(t.find_by_order(1) == "ababa");
	REQUIRE(t.find_by_order(2) == "ababa");
	REQUIRE(t.find_by_order(3) == "acba");
	t.erase("ababa");
	REQUIRE(t.count("ababa") == 1);
	REQUIRE(t.count_prefix("") == 3);
	REQUIRE(t.count_prefix("ab") == 2);
}
