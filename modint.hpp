/**
 * @file modint.hpp
 * @brief Class that wraps integer for modular arithmetic operations
 */

#ifndef MODINT_HPP
#define MODINT_HPP

#include <concepts>
#include <iostream>
#include <limits>
#include <type_traits>

template <class T>
concept modint_integer = std::same_as<T, int> || std::same_as<T, long long>;

template <modint_integer T = int, T MOD = 1'000'000'007> class modint;

template <modint_integer T, T MOD>
std::ostream &operator<<(std::ostream &, const modint<T, MOD> &);
template <modint_integer T, T MOD>
std::istream &operator>>(std::istream &, modint<T, MOD> &);

template <modint_integer T, T MOD> class modint {
  private:
	static_assert(MOD > 1);
	static_assert(MOD <= std::numeric_limits<T>::max() / 2);
	using mul_type =
	    std::conditional_t<std::same_as<T, int> || MOD <= 3'037'000'500LL,
	                       long long, __int128>;
	T x;

	static constexpr T normalize(T value) {
		if (value >= MOD) {
			value -= MOD;
			if (value < MOD)
				return value;
		} else if (value >= 0) {
			return value;
		} else {
			value += MOD;
			if (value >= 0)
				return value;
		}
		value %= MOD;
		return value < 0 ? value + MOD : value;
	}

  public:
	friend std::ostream &operator<< <T, MOD>(std::ostream &, const modint &);
	friend std::istream &operator>> <T, MOD>(std::istream &, modint &);

	constexpr modint(T value) : x(normalize(value)) {}
	template <std::signed_integral U>
	constexpr modint(const U &value)
	    : modint(static_cast<T>(value %
	                            static_cast<std::common_type_t<U, T>>(MOD))) {}
	constexpr modint() : x{} {}

	constexpr modint &operator+=(const modint &rhs) {
		x += rhs.x;
		if (x >= MOD)
			x -= MOD;
		return *this;
	}
	constexpr modint &operator++() { return *this += 1; }
	constexpr modint operator+(const modint &rhs) const {
		return modint(*this) += rhs;
	}
	constexpr modint operator++(int) {
		modint cpy(*this);
		++*this;
		return cpy;
	}

	constexpr modint &operator-=(const modint &rhs) {
		x -= rhs.x;
		if (x < 0)
			x += MOD;
		return *this;
	}
	constexpr modint &operator--() { return *this -= 1; }
	constexpr modint operator-(const modint &rhs) const {
		return modint(*this) -= rhs;
	}
	constexpr modint operator-() const { return modint() - *this; }
	constexpr modint operator--(int) {
		modint cpy(*this);
		--*this;
		return cpy;
	}

	constexpr modint &operator*=(const modint &rhs) {
		x = static_cast<T>(static_cast<mul_type>(x) * rhs.x % MOD);
		return *this;
	}
	constexpr modint operator*(const modint &rhs) const {
		return modint(*this) *= rhs;
	}

	constexpr modint pow(unsigned long long p) const {
		modint rt = 1, b = *this;
		for (; p; p >>= 1, b *= b)
			if (p & 1)
				rt *= b;
		return rt;
	}

	constexpr modint operator^(unsigned long long p) const { return pow(p); }
	constexpr modint &operator^=(unsigned long long p) {
		return *this = pow(p);
	}

	/** Requires prime MOD and a nonzero value. */
	constexpr modint inv() const { return pow(MOD - 2); }
	constexpr modint &operator/=(const modint &rhs) {
		return *this *= rhs.inv();
	}
	constexpr modint operator/(const modint &rhs) const {
		return modint(*this) /= rhs;
	}

	constexpr bool operator==(const modint &rhs) const { return x == rhs.x; }
	constexpr bool operator!=(const modint &rhs) const { return x != rhs.x; }

	constexpr explicit operator T() const { return x; }
	constexpr explicit operator bool() const { return bool(x); }
};

template <modint_integer T, T MOD>
std::ostream &operator<<(std::ostream &os, const modint<T, MOD> &arg) {
	return os << arg.x;
}
template <modint_integer T, T MOD>
std::istream &operator>>(std::istream &is, modint<T, MOD> &arg) {
	is >> arg.x;
	if (is)
		arg.x = modint<T, MOD>::normalize(arg.x);
	return is;
}

using mint_1097 = modint<>;
using mint_1099 = modint<int, 1'000'000'009>;
using mint_998 = modint<int, 998'244'353>;

namespace std {
template <modint_integer T, T MOD> struct hash<modint<T, MOD>> {
	size_t operator()(const modint<T, MOD> &s) const noexcept {
		return hash<T>{}(T(s));
	}
};
} // namespace std

#endif
