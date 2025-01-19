/**
 * @file functional.hpp
 * @brief Additonal functional structs and utilities not included in functional
 * library.
 */

#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include <algorithm>
#include <functional>
#include <numeric>

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
minimum(Comp, Proj) -> minimum<void, Proj, Comp>;

template <class T = void, class Proj = std::identity,
          class Comp = std::ranges::less>
class maximum {
  private:
	Comp comp;
	Proj proj;

  public:
	constexpr maximum(Comp comp = {}, Proj proj = {})
	    : comp(comp), proj(proj) {}
	constexpr T operator()(const T &lhs, const T &rhs) const {
		return std::ranges::max(lhs, rhs, comp, proj);
	}
};
template <class Proj, class Comp> class maximum<void, Proj, Comp> {
  private:
	Comp comp;
	Proj proj;

  public:
	constexpr maximum(Comp comp = {}, Proj proj = {})
	    : comp(comp), proj(proj) {}
	template <class T>
	constexpr T operator()(const T &lhs, const T &rhs) const {
		return std::ranges::max(lhs, rhs, comp, proj);
	}
};
template <class Comp, class Proj>
maximum(Comp, Proj) -> maximum<void, Proj, Comp>;

template <class T = void> struct gcd {
	constexpr T operator()(T lhs, T rhs) const { return std::gcd(lhs, rhs); }
};
template <> struct gcd<void> {
	template <class T1, class T2>
	constexpr auto operator()(T1 &&lhs, T2 &&rhs) const
	    -> decltype(std::gcd(std::forward<T1>(lhs), std::forward<T2>(rhs))) {
		return std::gcd(std::forward<T1>(lhs), std::forward<T2>(rhs));
	}
};

struct assign {
	template <class T1, class T2>
	constexpr T2 &&operator()(T1 &&, T2 &&t) const noexcept {
		return std::forward<T2>(t);
	}
};

struct noop {
	template <class T1, class T2>
	constexpr T1 &&operator()(T1 &&t, T2 &&) const noexcept {
		return std::forward<T1>(t);
	}
};

} // namespace fn

#endif
