/**
 * @file io.hpp
 * @brief IO utilities for common container types
 */

#ifndef IO_HPP
#define IO_HPP

#include <deque>
#include <functional>
#include <iostream>
#include <ranges>
#include <valarray>
#include <vector>

/**
 * IO helper functions.
 */
template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &r);
template <class T>
std::ostream &operator<<(std::ostream &os, const std::deque<T> &r);
template <class T>
std::ostream &operator<<(std::ostream &os, const std::valarray<T> &r);
template <class T>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::vector<T>> &r);
template <class T>
std::ostream &operator<<(std::ostream &os,
                         const std::valarray<std::valarray<T>> &r);
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::pair<T1, T2>> &r);
template <class... Ts>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::tuple<Ts...>> &r);
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p);
template <class... Ts>
std::ostream &operator<<(std::ostream &os, const std::tuple<Ts...> &t);
template <std::ranges::input_range R>
std::ostream &print_range(std::ostream &os, R &&r, char delimiter = ' ');
template <class T, std::size_t... Is>
std::ostream &print_tuple(std::ostream &os, const T &t,
                          std::index_sequence<Is...>);

template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &r) {
	return print_range(os, r);
}
template <class T>
std::ostream &operator<<(std::ostream &os, const std::deque<T> &r) {
	return print_range(os, r);
}
template <class T>
std::ostream &operator<<(std::ostream &os, const std::valarray<T> &r) {
	return print_range(os, r);
}
template <class T>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::vector<T>> &r) {
	return print_range(os, r, '\n');
}
template <class T>
std::ostream &operator<<(std::ostream &os,
                         const std::valarray<std::valarray<T>> &r) {
	return print_range(os, r, '\n');
}
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::pair<T1, T2>> &r) {
	return print_range(os, r, '\n');
}
template <class... Ts>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::tuple<Ts...>> &r) {
	return print_range(os, r, '\n');
}
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p) {
	return os << p.first << ' ' << p.second;
}
template <class... Ts>
std::ostream &operator<<(std::ostream &os, const std::tuple<Ts...> &t) {
	return print_tuple(os, t, std::index_sequence_for<Ts...>{});
}

template <std::ranges::input_range R>
std::ostream &print_range(std::ostream &os, R &&r, char delimiter) {
	for (auto it = std::ranges::begin(r); it != std::ranges::end(r); ++it) {
		if (it != std::ranges::begin(r))
			os << delimiter;
		os << *it;
	}
	return os;
}

template <class T, std::size_t... Is>
std::ostream &print_tuple(std::ostream &os, const T &t,
                          std::index_sequence<Is...>) {
	(..., (Is > 0 && (os << ' '), os << std::get<Is>(t)));
	return os;
}

template <class T, int base = 1> struct _put_indices {
	const T &v;
};
template <class T, int base = 1> _put_indices<T, base> put_indices(const T &v) {
	return _put_indices<T, base>{v};
}

template <class T, int base>
std::ostream &operator<<(std::ostream &os, _put_indices<T, base> pi);
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<T>, base> pi);
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::deque<T>, base> pi);
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::valarray<T>, base> pi);
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<std::vector<T>>, base> pi);
template <class T, int base>
std::ostream &
operator<<(std::ostream &os,
           _put_indices<std::valarray<std::valarray<T>>, base> pi);
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<std::pair<T, T>>, base> pi);
template <class... Ts, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<std::tuple<Ts...>>, base> pi);
template <class T1, class T2, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::pair<T1, T2>, base> pi);
template <class... Ts, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::tuple<Ts...>, base> pi);
template <class T, std::size_t... Is>
std::ostream &print_index_tuple(std::ostream &os, const T &t,
                                std::index_sequence<Is...>);

template <class T, int base>
std::ostream &operator<<(std::ostream &os, _put_indices<T, base> pi) {
	return os << pi.v + base;
}
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<T>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<T, base>(x);
	                       }));
}
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::valarray<T>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<T, base>(x);
	                       }));
}
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::deque<T>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<T, base>(x);
	                       }));
}
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<std::vector<T>>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<std::vector<T>, base>(x);
	                       }),
	                   '\n');
}
template <class T, int base>
std::ostream &
operator<<(std::ostream &os,
           _put_indices<std::valarray<std::valarray<T>>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<std::valarray<T>, base>(x);
	                       }),
	                   '\n');
}
template <class T, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::vector<std::pair<T, T>>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<std::pair<T, T>, base>(x);
	                       }),
	                   '\n');
}
template <class... Ts, int base>
std::ostream &
operator<<(std::ostream &os,
           _put_indices<std::vector<std::tuple<Ts...>>, base> pi) {
	return print_range(os, pi.v | std::views::transform([](const auto &x) {
		                       return put_indices<std::tuple<Ts...>, base>(x);
	                       }),
	                   '\n');
}
template <class T1, class T2, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::pair<T1, T2>, base> pi) {
	return os << put_indices<T1, base>(pi.v.first) << ' '
	          << put_indices<T2, base>(pi.v.second);
}
template <class... Ts, int base>
std::ostream &operator<<(std::ostream &os,
                         _put_indices<std::tuple<Ts...>, base> pi) {
	std::invoke(
	    [&os, &pi]<std::size_t... Is>(std::index_sequence<Is...>) {
		    (...,
		     (Is > 0 && (os << ' '),
		      os << put_indices<std::tuple_element_t<Is, std::tuple<Ts...>>,
		                        base>(std::get<Is>(pi.v))));
	    },
	    std::index_sequence_for<Ts...>{});
	return os;
}

template <class T>
std::istream &operator>>(std::istream &is, std::vector<T> &r);
template <class T> std::istream &operator>>(std::istream &is, std::deque<T> &r);
template <class T>
std::istream &operator>>(std::istream &is, std::valarray<T> &r);
template <class T1, class T2>
std::istream &operator>>(std::istream &is, std::pair<T1, T2> &x);
template <class... Ts>
std::istream &operator>>(std::istream &is, std::tuple<Ts...> &t);
template <class T, std::size_t... Is>
std::istream &read_tuple(std::istream &is, T &t, std::index_sequence<Is...>);

template <class T>
std::istream &operator>>(std::istream &is, std::vector<T> &r) {
	for (auto &x : r)
		is >> x;
	return is;
}
template <class T>
std::istream &operator>>(std::istream &is, std::deque<T> &r) {
	for (auto &x : r)
		is >> x;
	return is;
}
template <class T>
std::istream &operator>>(std::istream &is, std::valarray<T> &r) {
	for (auto &x : r)
		is >> x;
	return is;
}
template <class T1, class T2>
std::istream &operator>>(std::istream &is, std::pair<T1, T2> &x) {
	return is >> x.first >> x.second;
}
template <class... Ts>
std::istream &operator>>(std::istream &is, std::tuple<Ts...> &t) {
	return read_tuple(is, t, std::index_sequence_for<Ts...>{});
}

template <class T, std::size_t... Is>
std::istream &read_tuple(std::istream &is, T &t, std::index_sequence<Is...>) {
	(is >> ... >> std::get<Is>(t));
	return is;
}

template <class T, int base = 1> struct _get_indices {
	T &v;
};
template <class T, int base = 1> _get_indices<T, base> get_indices(T &v) {
	return _get_indices<T, base>{v};
}

template <class T, int base>
std::istream &operator>>(std::istream &is, _get_indices<T, base> gi);
template <class T, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::vector<T>, base> gi);
template <class T, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::deque<T>, base> gi);
template <class T, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::valarray<T>, base> gi);
template <class T1, class T2, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::pair<T1, T2>, base> gi);
template <class... Ts, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::tuple<Ts...>, base> gi);

template <class T, int base>
std::istream &operator>>(std::istream &is, _get_indices<T, base> gi) {
	is >> gi.v;
	gi.v -= base;
	return is;
}
template <class T, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::vector<T>, base> gi) {
	for (auto &x : gi.v)
		is >> get_indices<T, base>(x);
	return is;
}
template <class T, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::deque<T>, base> gi) {
	for (auto &x : gi.v)
		is >> get_indices<T, base>(x);
	return is;
}
template <class T, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::valarray<T>, base> gi) {
	for (auto &x : gi.v)
		is >> get_indices<T, base>(x);
	return is;
}
template <class T1, class T2, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::pair<T1, T2>, base> gi) {
	is >> get_indices<T1, base>(gi.v.first) >>
	    get_indices<T2, base>(gi.v.second);
	return is;
}
template <class... Ts, int base>
std::istream &operator>>(std::istream &is,
                         _get_indices<std::tuple<Ts...>, base> gi) {
	std::invoke(
	    [&is, &gi]<std::size_t... Is>(std::index_sequence<Is...>) {
		    ((is >>
		      get_indices<std::tuple_element_t<Is, std::tuple<Ts...>>, base>(
		          std::get<Is>(gi.v))),
		     ...);
	    },
	    std::index_sequence_for<Ts...>{});
	return is;
}

#endif
