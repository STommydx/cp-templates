/**
 * @file math.hpp
 * @brief Math utility functions
 */

#ifndef MATH_HPP
#define MATH_HPP

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

#endif
