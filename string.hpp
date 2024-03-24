/**
 * @file string.hpp
 * @brief String utilities
 */

#ifndef STRING_HPP
#define STRING_HPP

#include <array>
#include <cctype>
#include <memory>
#include <queue>
#include <ranges>
#include <vector>

#include "functional.hpp"
#include "utilities.hpp"

std::vector<int> prefix_function(std::ranges::random_access_range auto &&s) {
	std::vector<int> pi(std::ranges::size(s));
	for (int i = 1; i < std::ranges::ssize(s); i++) {
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

template <class T, class U = T, class UpdateOp = std::plus<>,
          class Charset = charset::lower>
class trie {
  protected:
	std::vector<std::array<size_t, Charset{}.size()>> tr;
	std::vector<T> dat;

	size_t root;
	UpdateOp op;
	Charset charset;

  public:
	static constexpr size_t npos = -1;

  protected:
	size_t create_node() {
		tr.emplace_back();
		tr.back().fill(0);
		dat.emplace_back();
		return tr.size() - 1;
	}

  public:
	trie() { root = create_node(); }

	T query(const std::string &s) const {
		size_t u = find(s);
		if (u == npos)
			return T();
		return dat[u];
	}

	void modify(const std::string &s, size_t val) {
		size_t u = root;
		for (char c : s) {
			size_t v = charset.to_index(c);
			if (!tr[u][v]) {
				tr[u][v] = create_node();
			}
			u = tr[u][v];
		}
		dat[u] = op(dat[u], val);
	}

	size_t find(const std::string &s) const {
		size_t u = root;
		for (char c : s) {
			size_t v = charset.to_index(c);
			if (!tr[u][v])
				return npos;
			u = tr[u][v];
		}
		return u;
	}
};

template <class Charset = charset::lower>
class ordered_trie : trie<size_t, size_t, std::plus<>, Charset> {
	using trie<size_t, size_t, std::plus<>, Charset>::tr;
	using trie<size_t, size_t, std::plus<>, Charset>::dat;
	using trie<size_t, size_t, std::plus<>, Charset>::root;
	using trie<size_t, size_t, std::plus<>, Charset>::charset;
	std::vector<size_t> cnt_prefix;

	size_t create_node() {
		auto r = trie<size_t, size_t, std::plus<>, Charset>::create_node();
		cnt_prefix.emplace_back();
		return r;
	}

	template <bool erase> void modify(const std::string &s, size_t val) {
		size_t u = root;
		if constexpr (erase)
			cnt_prefix[u] -= val;
		else
			cnt_prefix[u] += val;
		for (char c : s) {
			size_t v = charset.to_index(c);
			if (!tr[u][v]) {
				tr[u][v] = create_node();
			}
			u = tr[u][v];
			if constexpr (erase)
				cnt_prefix[u] -= val;
			else
				cnt_prefix[u] += val;
		}
		if constexpr (erase)
			dat[u] -= val;
		else
			dat[u] += val;
	}

  public:
	using trie<size_t, size_t, std::plus<>, Charset>::npos;

	ordered_trie() : trie<size_t, size_t, std::plus<>, Charset>() {
		cnt_prefix.emplace_back();
	}

	void insert(const std::string &s, size_t count = 1) {
		modify<false>(s, count);
	}

	void erase(const std::string &s, size_t count = 1) {
		modify<true>(s, count);
	}

	size_t count(const std::string &s) const {
		return trie<size_t, size_t, std::plus<>, Charset>::query(s);
	}

	size_t count_prefix(const std::string &s) const {
		auto u = trie<size_t, size_t, std::plus<>, Charset>::find(s);
		if (u == npos)
			return 0;
		return cnt_prefix[u];
	}

	size_t order_of_key(const std::string &s) const {
		size_t u = root;
		size_t ans = 0;
		for (char c : s) {
			ans += dat[u];
			size_t v = charset.to_index(c);
			for (size_t i = 0; i < v; i++) {
				if (tr[u][i])
					ans += cnt_prefix[tr[u][i]];
			}
			if (!tr[u][v])
				break;
			u = tr[u][v];
		}
		return ans;
	}

	std::string find_by_order(size_t k) const {
		size_t u = root;
		std::string ans;
		while (k < cnt_prefix[u]) {
			if (k < dat[u]) {
				break;
			}
			k -= dat[u];
			size_t i = 0;
			for (; i < charset.size(); i++) {
				if (tr[u][i]) {
					if (k < cnt_prefix[tr[u][i]]) {
						break;
					}
					k -= cnt_prefix[tr[u][i]];
				}
			}
			ans.push_back(charset.to_char(i));
			u = tr[u][i];
		}
		return ans;
	}

	size_t size() const { return cnt_prefix[root]; }
};

template <class Charset = charset::lower>
class ac_automation : trie<magic_vector<size_t>, size_t, std::plus<>, Charset> {
	size_t m;
	std::vector<size_t> fail;
	std::vector<size_t> topo_order;

	using trie<magic_vector<size_t>, size_t, std::plus<>, Charset>::tr;
	using trie<magic_vector<size_t>, size_t, std::plus<>, Charset>::dat;
	using trie<magic_vector<size_t>, size_t, std::plus<>, Charset>::root;
	using trie<magic_vector<size_t>, size_t, std::plus<>, Charset>::charset;

  public:
	ac_automation(const std::vector<std::string> &patterns)
	    : trie<magic_vector<size_t>, size_t, std::plus<>, Charset>(),
	      m(patterns.size()) {
		for (size_t i = 0; i < m; i++) {
			trie<magic_vector<size_t>, size_t, std::plus<>, Charset>::modify(
			    patterns[i], i);
		}
		// build failure graph
		fail.assign(tr.size(), 0);
		std::vector<size_t> in_degree(tr.size());
		std::queue<size_t> q;
		for (size_t i = 0; i < charset.size(); i++) {
			if (tr[root][i]) {
				fail[tr[root][i]] = root;
				q.push(tr[root][i]);
			}
		}
		while (!q.empty()) {
			size_t u = q.front();
			q.pop();
			for (size_t i = 0; i < charset.size(); i++) {
				if (tr[u][i]) {
					fail[tr[u][i]] = tr[fail[u]][i];
					in_degree[fail[tr[u][i]]]++;
					q.push(tr[u][i]);
				} else {
					tr[u][i] = tr[fail[u]][i];
				}
			}
		}
		// topological sort on failure graph
		for (size_t i = 0; i < tr.size(); i++) {
			if (in_degree[i] == 0) {
				q.push(i);
			}
		}
		while (!q.empty()) {
			size_t u = q.front();
			q.pop();
			topo_order.push_back(u);
			if (size_t v = fail[u]; !--in_degree[v]) {
				q.push(v);
			}
		}
	}

	size_t count_all_matches(const std::string &s) const {
		auto result = count_matches(s);
		size_t ans = 0;
		for (auto r : result) {
			ans += r > 0;
		}
		return ans;
	}

	std::vector<size_t> count_matches(const std::string &s) const {
		std::vector<size_t> match_cnt(tr.size());
		size_t u = root;
		for (char c : s) {
			int v = charset.to_index(c);
			u = tr[u][v];
			match_cnt[u]++;
		}
		std::vector<size_t> result(m);
		for (size_t u : topo_order) {
			for (size_t id : dat[u]) {
				result[id] += match_cnt[u];
			}
			size_t v = fail[u];
			match_cnt[v] += match_cnt[u];
		}
		return result;
	}
};

#endif
