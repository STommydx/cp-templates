/**
 * @file commons.hpp
 * @brief Provides abbreviations and other useful functions.
 */

#ifndef COMMONS_HPP
#define COMMONS_HPP

#include <iostream>
#include <limits>
#include <queue>
#include <ranges>
#include <utility>
#include <vector>

/**
 * Abbreviations for common types.
 */
using vi = std::vector<int>;
using vvi = std::vector<vi>;
using vvvi = std::vector<vvi>;
using pii = std::pair<int, int>;
using vpii = std::vector<pii>;
using ll = long long;
using vll = std::vector<long long>;
using vvll = std::vector<vll>;
using vvvll = std::vector<vvll>;
using pll = std::pair<ll, ll>;
using vpll = std::vector<pll>;

/**
 * Abbreviation for the STL priority_queue with smaller element at the top.
 */
template <class T>
using min_queue = std::priority_queue<T, std::vector<T>, std::greater<T>>;

/**
 * Abbreviation for infinity constant.
 */
template <class T = int>
constexpr T INFTY =
    std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity()
                                         : std::numeric_limits<T>::max();
template <> constexpr int INFTY<int> = 0x3f3f3f3f;
template <> constexpr ll INFTY<ll> = 0x3f3f3f3f3f3f3f3fLL;
template <class T, class U>
constexpr std::pair<T, U> INFTY<std::pair<T, U>>{INFTY<T>, INFTY<U>};
[[maybe_unused]] constexpr int INF = INFTY<>;
[[maybe_unused]] constexpr ll BINF = INFTY<ll>;

/**
 * IO helper functions.
 */
template <std::ranges::input_range R>
std::ostream &print_range(std::ostream &os, R &&r, char delimiter = ' ');
template <class T, std::size_t... Is>
std::ostream &print_tuple(std::ostream &os, const T &t,
                          std::index_sequence<Is...>);

template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &r) {
	return print_range(os, r);
}
template <class T>
std::ostream &operator<<(std::ostream &os, const std::deque<T> &r) {
	return print_range(os, r);
}
template <class T>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::vector<T>> &r) {
	return print_range(os, r, '\n');
}
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::pair<T1, T2>> &r) {
	return print_range(os, r, '\n');
}
template <class... Ts>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::tuple<Ts...>> &r) {
	return print_range(os, r, '\n');
}
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p) {
	return os << p.first << ' ' << p.second;
}
template <class... Ts>
std::ostream &operator<<(std::ostream &os, const std::tuple<Ts...> &t) {
	return print_tuple(os, t, std::index_sequence_for<Ts...>{});
}

template <std::ranges::input_range R>
std::ostream &print_range(std::ostream &os, R &&r, char delimiter) {
	for (auto it = std::ranges::begin(r); it != std::ranges::end(r); ++it) {
		if (it != std::ranges::begin(r))
			os << delimiter;
		os << *it;
	}
	return os;
}

template <class T, std::size_t... Is>
std::ostream &print_tuple(std::ostream &os, const T &t,
                          std::index_sequence<Is...>) {
	(..., (Is > 0 && (os << ' '), os << std::get<Is>(t)));
	return os;
}

template <class T, std::size_t... Is>
std::istream &read_tuple(std::istream &is, T &t, std::index_sequence<Is...>);

template <class T>
std::istream &operator>>(std::istream &is, std::vector<T> &r) {
	for (auto &x : r)
		is >> x;
	return is;
}
template <class T>
std::istream &operator>>(std::istream &is, std::deque<T> &r) {
	for (auto &x : r)
		is >> x;
	return is;
}
template <class T1, class T2>
std::istream &operator>>(std::istream &is, std::pair<T1, T2> &x) {
	return is >> x.first >> x.second;
}
template <class... Ts>
std::istream &operator>>(std::istream &is, std::tuple<Ts...> &t) {
	return read_tuple(is, t, std::index_sequence_for<Ts...>{});
}

template <class T, std::size_t... Is>
std::istream &read_tuple(std::istream &is, T &t, std::index_sequence<Is...>) {
	(is >> ... >> std::get<Is>(t));
	return is;
}

#endif
