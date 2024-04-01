/**
 * @file limits.hpp
 * @brief Infinity constants for competitive programming purposes
 */

#ifndef LIMITS_HPP
#define LIMITS_HPP

#include <limits>

template <class T> struct cp_limits {
	static constexpr T infinity() {
		return std::numeric_limits<T>::has_infinity
		           ? std::numeric_limits<T>::infinity()
		           : std::numeric_limits<T>::max() / 2;
	}
	static constexpr T negative_infinity() { return -infinity(); }
};

template <> struct cp_limits<int> {
	static constexpr int infinity() { return 0x3f3f3f3f; }
	static constexpr int negative_infinity() { return -infinity(); }
};

template <> struct cp_limits<long long> {
	static constexpr long long infinity() { return 0x3f3f3f3f3f3f3f3fLL; }
	static constexpr long long negative_infinity() { return -infinity(); }
};

template <class T1, class T2> struct cp_limits<std::pair<T1, T2>> {
	static constexpr std::pair<T1, T2> infinity() {
		return {cp_limits<T1>::infinity(), cp_limits<T2>::infinity()};
	}
	static constexpr std::pair<T1, T2> negative_infinity() {
		return {cp_limits<T1>::negative_infinity(),
		        cp_limits<T2>::negative_infinity()};
	}
};

#endif
