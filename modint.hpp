/**
 * @file modint.hpp
 * @brief Class that warps integer for modular arithmetic operations
 */

#ifndef MODINT_HPP
#define MODINT_HPP

#include <iostream>

template <std::signed_integral T = int, T MOD = 1'000'000'007> class modint;

template <std::signed_integral T, T MOD>
std::ostream &operator<<(std::ostream &, const modint<T, MOD> &);
template <std::signed_integral T, T MOD>
std::istream &operator>>(std::istream &, modint<T, MOD> &);

template <std::signed_integral T, T MOD> class modint {
  private:
	T x;

  public:
	friend std::ostream &operator<< <T, MOD>(std::ostream &, const modint &);
	friend std::istream &operator>> <T, MOD>(std::istream &, modint &);
	constexpr modint(const T &x) : x(x) {
		if (this->x >= MOD)
			this->x -= MOD;
		if (this->x < 0)
			this->x += MOD;
	}
	constexpr modint(const std::signed_integral auto &x) : modint(T(x % MOD)) {}
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
		x = 1LL * x * rhs.x % MOD;
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

template <std::signed_integral T, T MOD>
std::ostream &operator<<(std::ostream &os, const modint<T, MOD> &arg) {
	return os << arg.x;
}
template <std::signed_integral T, T MOD>
std::istream &operator>>(std::istream &is, modint<T, MOD> &arg) {
	is >> arg.x;
	if (arg.x >= MOD)
		arg.x -= MOD;
	if (arg.x < 0)
		arg.x += MOD;
	return is;
}

using mint_1097 = modint<>;
using mint_1099 = modint<int, 1'000'000'009>;
using mint_998 = modint<int, 998'244'353>;

namespace std {
template <class T, T MOD> struct hash<modint<T, MOD>> {
	size_t operator()(const modint<T, MOD> &s) const noexcept {
		return hash<T>{}(T(s));
	}
};
} // namespace std

#endif
