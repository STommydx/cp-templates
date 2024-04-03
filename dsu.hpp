/**
 * @file dsu.hpp
 * @brief Disjoint set union data structure
 */

#ifndef DSU_HPP
#define DSU_HPP

#include <vector>

template <class T = void, class Op = std::plus<>> class dsu;

template <class Op> class dsu<void, Op> : std::vector<int> {
  public:
	static const int same_set = -1;

	explicit dsu(size_t n) : std::vector<int>(n, -1) {}
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
	size_t size() const { return std::vector<int>::size(); }
};

template <class T, class Op> class dsu : public dsu<void, Op> {
	std::vector<T> dat;
	Op op;

  public:
	using dsu<void, Op>::same_set;

	explicit dsu(const std::vector<T> &v, const Op &op = {})
	    : dsu<void, Op>(v.size()), dat(v), op(op) {}
	explicit dsu(size_t n, const T &init = {}, const Op &op = {})
	    : dsu(std::vector<T>(n, init), op) {}

	using dsu<void, Op>::find_set;
	T operator[](int x) { return dat[find_set(x)]; }
	int union_sets(int u, int v) {
		int gu = find_set(u), gv = find_set(v);
		int result = dsu<void, Op>::union_sets(gu, gv);
		if (result != same_set)
			dat[result] = op(dat[result], dat[result ^ gu ^ gv]);
		return result;
	}
};

#endif
