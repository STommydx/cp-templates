/**
 * @file mo.hpp
 * @brief MO's algorithm helper for answering queries in O(n sqrt(n))
 */

#ifndef MO_HPP
#define MO_HPP

#include <algorithm>
#include <bit>
#include <numeric>
#include <utility>
#include <vector>

class mo {
  public:
	using query = std::pair<int, int>;

  private:
	int n, magic;
	std::vector<query> queries;

	constexpr int block(int x) const { return x >> magic; }

  public:
	explicit mo(int n, const std::vector<query> &queries = {})
	    : n(n), magic(std::bit_width<unsigned int>(n) / 2), queries(queries) {}

	size_t push_query(int l, int r) {
		queries.emplace_back(l, r);
		return queries.size() - 1;
	}

	template <class T, std::invocable<int, int, size_t> Answer,
	          std::invocable<int> PushLeft, std::invocable<int> PopLeft,
	          std::invocable<int> PushRight = PushLeft,
	          std::invocable<int> PopRight = PopLeft>
	    requires std::indirectly_writable<
	        typename std::vector<T>::iterator,
	        std::invoke_result_t<Answer, int, int, size_t>>
	std::vector<T> solve(Answer answer = {}, PushLeft push_left = {},
	                     PopLeft pop_left = {}, PushRight push_right = {},
	                     PopRight pop_right = {}) {
		std::vector<size_t> query_order(queries.size());
		std::iota(query_order.begin(), query_order.end(), 0);
		std::ranges::sort(query_order, {}, [&](int i) {
			auto &[ql, qr] = queries[i];
			return std::make_pair(block(ql), block(ql) ? qr : -qr);
		});
		std::vector<T> result(queries.size());
		int l = 0, r = -1;
		for (size_t q : query_order) {
			auto [ql, qr] = queries[q];
			while (l > ql) {
				push_left(--l);
			}
			while (r < qr) {
				push_right(++r);
			}
			while (l < ql) {
				pop_left(l++);
			}
			while (r > qr) {
				pop_right(r--);
			}
			result[q] = answer(l, r, q);
		}
		return result;
	}
	template <class T, std::invocable<int, int, size_t> Answer,
	          std::invocable<> Save, std::invocable<> Restore,
	          std::invocable<int> PushLeft,
	          std::invocable<int> PushRight = PushLeft>
	    requires std::indirectly_writable<
	        typename std::vector<T>::iterator,
	        std::invoke_result_t<Answer, int, int, size_t>>
	std::vector<T> solve_rollback(Answer answer = {}, Save save = {},
	                              Restore restore = {}, PushLeft push_left = {},
	                              PushRight push_right = {}) {
		std::vector<size_t> query_order(queries.size());
		std::iota(query_order.begin(), query_order.end(), 0);
		std::ranges::sort(query_order, {}, [&](int i) {
			auto &[ql, qr] = queries[i];
			return std::make_pair(block(ql), qr);
		});
		std::vector<T> result(queries.size());
		int last_block = -1;
		int l = 0, r = -1;
		save(); // save empty state
		for (int qi : query_order) {
			auto [ql, qr] = queries[qi];
			if (block(ql) != last_block) {
				// rollback everything and create save point again
				restore();
				save();
				// set boundary at first element of next block
				l = (block(ql) + 1) << magic;
				r = l - 1;
			}
			if (block(ql) == block(qr)) {
				// brute force calculation
				// current state should be empty as we are in a new block
				// (rollbacked by the last if condition)
				save();
				for (int j = ql; j <= qr; j++)
					push_right(j);
				result[qi] = answer(l, r, qi);
				restore();
				continue;
			}
			while (r < qr)
				push_right(++r);
			save();
			while (l > ql)
				push_left(--l);
			result[qi] = answer(l, r, qi);
			restore();
		}
		return result;
	}
};

#endif
