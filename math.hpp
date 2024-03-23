/**
 * @file math.hpp
 * @brief Math utility functions
 */

#ifndef MATH_HPP
#define MATH_HPP

#include <array>
#include <bit>
#include <concepts>

template <std::unsigned_integral T> constexpr T isqrt(T x) {
	T guess = 0;
	for (int j = std::bit_width(x) / 2; j >= 0; j--) {
		if (T new_guess = guess + (T(1) << j); new_guess * new_guess <= x) {
			guess = new_guess;
		}
	}
	return guess;
}

template <std::signed_integral T> constexpr T isqrt(T x) {
	return isqrt(static_cast<std::make_unsigned_t<T>>(x));
}

/**
 * Compare two fractions in the form of `a/b` and `c/d` without overflows.
 * Modified from https://codeforces.com/blog/entry/21588?#comment-262867
 */
template <std::integral T> constexpr auto fraction_cmp(T a, T b, T c, T d) {
	if (b < 0)
		a = -a, b = -b;
	if (d < 0)
		c = -c, d = -d;
	if (a / b != c / d)
		return a / b <=> c / d;
	a %= b, c %= d;
	if (a == 0 || c == 0)
		return a <=> c;
	return fraction_cmp(d, c, b, a);
}

template <int N> class prime_sieve {
	std::array<int, N> min_prime_factor;

  public:
	constexpr prime_sieve() {
		min_prime_factor.fill(0);
		for (int i = 2; i < N; i++) {
			if (!min_prime_factor[i]) {
				min_prime_factor[i] = i;
				if (1LL * i * i >= N)
					continue;
				for (int j = i * i; j < N; j += i)
					if (!min_prime_factor[j])
						min_prime_factor[j] = i;
			}
		}
	}
	constexpr int operator[](int n) const { return min_prime_factor[n]; }
	constexpr bool is_prime(int n) const { return min_prime_factor[n] == n; }
};

#endif
