/**
 * @file string.hpp
 * @brief String utilities
 */

#ifndef STRING_HPP
#define STRING_HPP

#include <ranges>
#include <vector>

std::vector<int> prefix_function(std::ranges::random_access_range auto &&s) {
	std::vector<int> pi(std::ranges::size(s));
	for (int i = 1; i < std::ranges::size(s); i++) {
		int j = pi[i - 1];
		while (j > 0 && s[i] != s[j])
			j = pi[j - 1];
		if (s[i] == s[j])
			j++;
		pi[i] = j;
	}
	return pi;
}

std::vector<int> kmp(const std::string &str, const std::string &pattern) {
	std::vector<int> pi = prefix_function(pattern + "#" + str);
	std::vector<int> result;
	auto m = std::ranges::ssize(pattern);
	for (int i = m + 1; i < std::ranges::ssize(pi); i++) {
		if (pi[i] == m) {
			result.push_back(i - m - m);
		}
	}
	return result;
}

#endif
