/**
 * @file math.hpp
 * @brief Math utility functions
 */

#ifndef MATH_HPP
#define MATH_HPP

#include <array>
#include <bit>
#include <bitset>
#include <concepts>
#include <vector>

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

template <size_t N> class prime_sieve {
	std::array<unsigned int, N> min_prime_factor;

  public:
	constexpr prime_sieve() {
		min_prime_factor.fill(0);
		for (size_t i = 2; i < N; i++) {
			if (!min_prime_factor[i]) {
				min_prime_factor[i] = i;
				if (1uLL * i * i >= N)
					continue;
				for (size_t j = i * i; j < N; j += i)
					if (!min_prime_factor[j])
						min_prime_factor[j] = i;
			}
		}
	}
	constexpr int operator[](size_t n) const { return min_prime_factor[n]; }
	constexpr bool is_prime(size_t n) const { return min_prime_factor[n] == n; }
	constexpr std::vector<int> primes() const {
		std::vector<int> primes;
		for (size_t i = 2; i < N; i++)
			if (is_prime(i))
				primes.push_back(i);
		return primes;
	}
};

template <size_t N> class prime_sieve_bitset {
	std::bitset<N> prime;

  public:
	// note that member functions of std::bitset are constexpr since C++23
	prime_sieve_bitset() {
		prime.set();
		prime[0] = prime[1] = false;
		for (size_t i = 2; i < N; i++) {
			if (prime[i]) {
				if (1uLL * i * i >= N)
					continue;
				for (size_t j = i * i; j < N; j += i)
					prime[j] = false;
			}
		}
	}
	bool operator[](size_t n) const { return prime[n]; }
	bool is_prime(size_t n) const { return prime[n]; }
	std::vector<int> primes() const {
		std::vector<int> primes;
		for (size_t i = 0; i < N; i++)
			if (prime[i])
				primes.push_back(i);
		return primes;
	}
};

#endif
