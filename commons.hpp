/**
 * @file commons.hpp
 * @brief Provides abbreviations and other useful functions.
 */

#ifndef COMMONS_HPP
#define COMMONS_HPP

#include <utility>
#include <vector>
#include <queue>
#include <limits>

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
template <class T> using min_queue = std::priority_queue<T, std::vector<T>, std::greater<T>>;

/**
 * Abbreviation for infinity constant.
 */
template <class T = int> constexpr T INFTY = std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max();
template <> constexpr int INFTY<int> = 0x3f3f3f3f;
template <> constexpr ll INFTY<ll> = 0x3f3f3f3f3f3f3f3fLL;
template <class T, class U> constexpr std::pair<T, U> INFTY<std::pair<T, U>>{INFTY<T>, INFTY<U>};
constexpr int INF = INFTY<>;
constexpr ll BINF = INFTY<ll>;

#endif
