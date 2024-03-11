/**
 * @file dsu.hpp
 * @brief Disjoint set union data structure
 */

#ifndef DSU_HPP
#define DSU_HPP

#include <vector>

class dsu : std::vector<int> {
  public:
	static const int same_set = -1;

	dsu(size_t n) : std::vector<int>(n, -1) {}
	int find_set(int x) {
		return (*this)[x] >= 0 ? (*this)[x] = find_set((*this)[x]) : x;
	}
	int size_of_set(int x) { return -(*this)[find_set(x)]; }
	int union_sets(int u, int v) {
		int gu = find_set(u), gv = find_set(v);
		if (gu == gv)
			return same_set;
		if ((*this)[gu] < (*this)[gv])
			std::swap(gu, gv);
		(*this)[gv] += (*this)[gu];
		(*this)[gu] = gv;
		return gv;
	}
};

#endif
