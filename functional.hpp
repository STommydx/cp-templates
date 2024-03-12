/**
 * @file functional.hpp
 * @brief Additonal functional structs and utilities not included in functional
 * library.
 */

#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include <algorithm>
#include <functional>
#include <ranges>

namespace fn {
template <class T = void, class Proj = std::identity,
          class Comp = std::ranges::less>
class minimum {
  private:
	Comp comp;
	Proj proj;

  public:
	constexpr minimum(Comp comp = {}, Proj proj = {})
	    : comp(comp), proj(proj) {}
	constexpr T operator()(const T &lhs, const T &rhs) const {
		return std::ranges::min(lhs, rhs, comp, proj);
	}
};
template <class Proj, class Comp> class minimum<void, Proj, Comp> {
  private:
	Comp comp;
	Proj proj;

  public:
	constexpr minimum(Comp comp = {}, Proj proj = {})
	    : comp(comp), proj(proj) {}
	template <class T>
	constexpr T operator()(const T &lhs, const T &rhs) const {
		return std::ranges::min(lhs, rhs, comp, proj);
	}
};
template <class Comp, class Proj>
minimum(Comp, Proj) -> minimum<void, std::identity, std::ranges::less>;
} // namespace fn

#endif