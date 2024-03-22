/**
 * @file cht.hpp
 * @brief Convex hull trick implementation
 */

#ifndef CHT_HPP
#define CHT_HPP

#include "math.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>

template <class T> class cht {
  public:
	using line = std::pair<T, T>;
	static constexpr size_t auto_assign = std::numeric_limits<size_t>::max();

  private:
	size_t n;
	std::vector<line> hull;
	std::vector<size_t> hull_line_idx;

	/**
	 * Check intersection of l1 and l2, l1 and l3. If the intersection of l1 and
	 * l3 has x coordinate not smaller than the intersection of l1 and l2, then
	 * line l2 should be skipped in the hull.
	 */
	static bool should_skip(const line &l1, const line &l2, const line &l3) {
		return fraction_cmp(l3.second - l1.second, l1.first - l3.first,
		                    l2.second - l1.second, l1.first - l2.first) >= 0;
	}

  public:
	cht() = default;
	explicit cht(const std::vector<line> &lines) : n(lines.size()) {
		std::vector<size_t> idx(lines.size());
		std::iota(idx.begin(), idx.end(), 0);
		auto indexer = [&](size_t idx) { return lines[idx].first; };
		if (!std::ranges::is_sorted(idx, std::ranges::less{}, indexer)) {
			std::ranges::sort(idx, std::ranges::less{}, indexer);
		}
		for (size_t i : idx) {
			add_line(lines[i], i);
		}
	}

	void add_line(const line &l, size_t line_idx = auto_assign) {
		while (hull.size() >= 2 &&
		       should_skip(hull[hull.size() - 2], hull[hull.size() - 1], l)) {
			hull.pop_back();
			hull_line_idx.pop_back();
		}
		hull.push_back(l);
		hull_line_idx.push_back(line_idx == auto_assign ? n++ : line_idx);
	}

	std::pair<T, size_t> query(T x) const {
		auto idx = *std::ranges::lower_bound(
		    std::views::iota(size_t{0}, hull.size() + 1), true,
		    std::ranges::less{}, [&](size_t mi) {
			    if (mi + 1 >= hull.size())
				    return true;
			    // check intersection of lines hull[mi] and hull[mi + 1]
			    // return whether x is not smaller than the x coordinate of
			    // the intersection
			    return fraction_cmp(x, T{1},
			                        hull[mi].second - hull[mi + 1].second,
			                        hull[mi + 1].first - hull[mi].first) >= 0;
		    });
		return {hull[idx].first * x + hull[idx].second, hull_line_idx[idx]};
	}

	size_t size() const { return hull.size(); }
};

#endif
