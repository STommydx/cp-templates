/**
 * @file string.hpp
 * @brief String utilities
 */

#ifndef STRING_HPP
#define STRING_HPP

#include <array>
#include <cctype>
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

namespace charset {
struct lower {
	constexpr size_t to_index(char c) const { return c - 'a'; }
	constexpr char to_char(size_t i) const { return 'a' + i; }
	constexpr size_t size() const { return 26; }
};
struct upper {
	constexpr size_t to_index(char c) const { return c - 'A'; }
	constexpr char to_char(size_t i) const { return 'A' + i; }
	constexpr size_t size() const { return 26; }
};
struct digit {
	constexpr size_t to_index(char c) const { return c - '0'; }
	constexpr char to_char(size_t i) const { return '0' + i; }
	constexpr size_t size() const { return 10; }
};
struct alpha {
	constexpr size_t to_index(char c) const {
		if (::std::isupper(c))
			return c - 'A';
		return c - 'a' + 26;
	}
	constexpr char to_char(size_t i) const {
		if (i < 26)
			return 'A' + i;
		return 'a' + i - 26;
	}
	constexpr size_t size() const { return 52; }
};
struct alnum {
	constexpr size_t to_index(char c) const {
		if (::std::isdigit(c))
			return c - '0';
		if (::std::isupper(c))
			return c - 'A' + 10;
		return c - 'a' + 36;
	}
	constexpr char to_char(size_t i) const {
		if (i < 10)
			return '0' + i;
		if (i < 36)
			return 'A' + i - 10;
		return 'a' + i - 36;
	}
	constexpr size_t size() const { return 62; }
};
}; // namespace charset

template <class Charset = charset::lower> class trie {
	struct trie_node {
		size_t count = 0;
		size_t count_prefix = 0;
		std::array<size_t, Charset{}.size()> child = {};
	};

	std::vector<trie_node> tr;
	size_t root;
	Charset charset;

	size_t create_node() {
		tr.emplace_back();
		return tr.size() - 1;
	}

	template <bool erase> void modify(const std::string &s, size_t val) {
		size_t u = root;
		if constexpr (erase)
			tr[u].count_prefix -= val;
		else
			tr[u].count_prefix += val;
		for (char c : s) {
			size_t v = charset.to_index(c);
			if (!tr[u].child[v]) {
				tr[u].child[v] = create_node();
			}
			u = tr[u].child[v];
			if constexpr (erase)
				tr[u].count_prefix -= val;
			else
				tr[u].count_prefix += val;
		}
		if constexpr (erase)
			tr[u].count -= val;
		else
			tr[u].count += val;
	}

  public:
	trie() { root = create_node(); }

	void insert(const std::string &s, size_t count = 1) {
		modify<false>(s, count);
	}

	void erase(const std::string &s, size_t count = 1) {
		modify<true>(s, count);
	}

	size_t count(const std::string &s) const {
		size_t u = root;
		for (char c : s) {
			size_t v = charset.to_index(c);
			if (!tr[u].child[v])
				return 0;
			u = tr[u].child[v];
		}
		return tr[u].count;
	}

	size_t count_prefix(const std::string &s) const {
		size_t u = root;
		for (char c : s) {
			size_t v = charset.to_index(c);
			if (!tr[u].child[v])
				return 0;
			u = tr[u].child[v];
		}
		return tr[u].count_prefix;
	}

	size_t order_of_key(const std::string &s) const {
		size_t u = root;
		size_t ans = 0;
		for (char c : s) {
			ans += tr[u].count;
			size_t v = charset.to_index(c);
			for (size_t i = 0; i < v; i++) {
				if (tr[u].child[i])
					ans += tr[tr[u].child[i]].count_prefix;
			}
			if (!tr[u].child[v])
				break;
			u = tr[u].child[v];
		}
		return ans;
	}

	std::string find_by_order(size_t k) const {
		size_t u = root;
		std::string ans;
		while (k < tr[u].count_prefix) {
			if (k < tr[u].count) {
				break;
			}
			k -= tr[u].count;
			size_t i = 0;
			for (; i < charset.size(); i++) {
				if (tr[u].child[i]) {
					if (k < tr[tr[u].child[i]].count_prefix) {
						break;
					}
					k -= tr[tr[u].child[i]].count_prefix;
				}
			}
			ans.push_back(charset.to_char(i));
			u = tr[u].child[i];
		}
		return ans;
	}

	size_t size() const { return tr[root].count_prefix; }
};

#endif
