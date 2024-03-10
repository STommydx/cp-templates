#include "commons.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

TEST_CASE("abbreviations are defined", "[commons]") {
    SECTION("long long datatype") {
        ll x = 1LL << 40;
        REQUIRE(x == 1LL << 40);
    }
    SECTION("mulit-dimensional vectors") {
        vvvi three_d_matrix(1, vvi(1, vi(1, 1)));
        REQUIRE(three_d_matrix[0][0][0] == 1);
        vvvll three_d_ll_matrix(1, vvll(1, vll(1, 1LL << 40)));
        REQUIRE(three_d_ll_matrix[0][0][0] == 1LL << 40);
    }
    SECTION("pairs") {
        vpii vpii_vector(1, pii(1, 2));
        vpii_vector.emplace_back(3, 4);
        REQUIRE(vpii_vector[0].first == 1);
        REQUIRE(vpii_vector[1].second == 4);
        vpll vpll_vector(1, pll(1LL << 40, 2));
        vpll_vector.emplace_back(3, 4);
        REQUIRE(vpll_vector[0].first == 1LL << 40);
        REQUIRE(vpll_vector[1].second == 4);
    }
}

TEST_CASE("min_queue behaves as expected", "[commons]") {
    min_queue<int> q;
    q.push(5);
    q.push(1);
    q.push(3);
    REQUIRE(q.top() == 1);
    q.pop();
    REQUIRE(q.top() == 3);
}

TEST_CASE("infinity constant is defined properly", "[commons]") {
    SECTION("infinity is defined for mulitple types") {
        int int_inf = INFTY<int>;
        REQUIRE(int_inf > 0);
        ll ll_inf = INFTY<ll>;
        REQUIRE(ll_inf > 0LL);
        std::pair<int, ll> pair_inf = INFTY<std::pair<int, ll>>;
        REQUIRE(pair_inf.first > 0);
        REQUIRE(pair_inf.second > 0LL);
        double double_inf = INFTY<double>;
        REQUIRE(double_inf > 1e200);
    }
    SECTION("infinity constants is defined for integer types") {
        REQUIRE(INF == INFTY<int>);
        REQUIRE(BINF == INFTY<ll>);
    }
    SECTION("double of integer infinity does not overflow") {
        int double_inf = INF + INF;
        REQUIRE(double_inf > INF);
        ll double_ll_inf = BINF + BINF;
        REQUIRE(double_ll_inf > BINF);
    }
}

TEST_CASE("IO operator overload works for common cp types", "[commons]") {
    SECTION("vector outputs elements separated by space") {
        std::stringstream ss;
        std::vector<std::string> v{"1", "22", "333"};
        ss << v;
        REQUIRE(ss.str() == "1 22 333");
    }
    SECTION("2D vector outputs elements separated by space and newline") {
        std::stringstream ss;
        std::vector<std::vector<std::string>> v{{"1", "22", "333"}, {"4444", "55555", "666666"}};
        ss << v;
        REQUIRE(ss.str() == "1 22 333\n4444 55555 666666");
    }
    SECTION("pair outputs elements separated by space") {
        std::stringstream ss;
        std::pair<std::string, int> p{"1", 22};
        ss << p;
        REQUIRE(ss.str() == "1 22");
    }
    SECTION("tuple outputs elements separated by space") {
        std::stringstream ss;
        std::tuple<std::string, int, double> t{"1", 22, 3.3};
        ss << t;
        REQUIRE(ss.str() == "1 22 3.3");
    }
    SECTION("vector of pairs outputs elements separated by space and newline") {
        std::stringstream ss;
        std::vector<std::pair<std::string, int>> v{{"1", 22}, {"4444", 55555}, {"666666", 7777777}};
        ss << v;
        REQUIRE(ss.str() == "1 22\n4444 55555\n666666 7777777");
    }
    SECTION("vector of tuples outputs elements separated by space and newline") {
        std::stringstream ss;
        std::vector<std::tuple<std::string, int, double>> v{{"1", 22, 3.3}, {"4444", 55555, 6.6}, {"666666", 7777777, 8.8}};
        ss << v;
        REQUIRE(ss.str() == "1 22 3.3\n4444 55555 6.6\n666666 7777777 8.8");
    }
}
