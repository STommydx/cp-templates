#include "modint.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <vector>

TEST_CASE("typical 1e9 + 7 modding arithmetic", "[modint]") {
	using mint = mint_1097;

	SECTION("constructors and int conversions") {
		REQUIRE(int(mint(5)) == 5);
		REQUIRE(int(mint(-1)) == 1'000'000'006);
		REQUIRE(int(mint(3'000'000'022LL)) == 1);
		REQUIRE(int(mint(-3'000'000'020LL)) == 1);
		REQUIRE(int(mint()) == 0);
		REQUIRE(mint(3) == -1'000'000'004);
		REQUIRE(bool(mint(5)) == true);
		REQUIRE(bool(mint(0)) == false);
		mint x = 3;
		REQUIRE(int(x) == 3);
		x = -3;
		REQUIRE(int(x) == 1'000'000'004);
		std::vector<mint> v(3, 2);
		REQUIRE(int(v[0]) == 2);
		std::vector<mint> v0(5);
		REQUIRE(bool(v0[0]) == false);
	}

	SECTION("arithmetic operations and comparisons") {
		mint x = 5, y = 4, z = 1'000'000'005;
		REQUIRE(x + y == 9);
		REQUIRE(y - x == -1);
		REQUIRE(x - y == z + 3);
		REQUIRE(x - z == y + 3);
		REQUIRE(x * y == 20);
		REQUIRE((y - x) * (y + x) == y * y - x * x);
		REQUIRE((y - z) * (y - z) == y * y - y * z * 2 + z * z);
	}

	SECTION("power and modular inverse") {
		mint x = 2, y = 5;
		REQUIRE(x.inv() == 500'000'004);
		REQUIRE(y * y.inv() == 1);
		REQUIRE(y.pow(3) == 125);
		REQUIRE(x / y == (y / x).inv());
		REQUIRE((x ^ 15) / (y ^ 4) == ((x / y) ^ 4) * (x ^ 11));
	}
}

TEST_CASE("modint for unusual modulos", "[modint]") {
	SECTION("modulo with small prime") {
		using m5 = modint<int, 5>;
		m5 a = 2, b = 11LL; // only numbers from [-MOD, MOD + MOD) is supported
		                    // for mod_int(T) constructor
		REQUIRE(b == 1);
		REQUIRE(a + a == b - 2);
		REQUIRE(a * a == a + b * 2);
		REQUIRE((a ^ 15) / (b ^ 4) == ((a / b) ^ 4) * (a ^ 11));
	}
	SECTION("modulo with long long") {
		using mll = modint<long long, 2'382'877'633LL>;
		mll a = 2LL, b = 2'382'877'634LL;
		REQUIRE(b == 1);
		REQUIRE(a - 3 == b - 2);
		REQUIRE(a * a == a + b * 2);
		REQUIRE((a ^ 15) / (b ^ 4) == ((a / b) ^ 4) * (a ^ 11));
	}
}

TEST_CASE("IO for modint", "[modint]") {
	using mint = mint_1099;
	SECTION("input operator overloading") {
		std::stringstream ss{"-1"};
		mint a;
		ss >> a;
		REQUIRE(a == -1);
	}
	SECTION("output operator overloading") {
		std::stringstream ss;
		ss << mint(5);
		REQUIRE(ss.str() == "5");
	}
}
