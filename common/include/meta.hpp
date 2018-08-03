
#pragma once

#include <boost/callable_traits/args.hpp>
#include <boost/callable_traits/return_type.hpp>
#include <utility>
#include <tuple>


namespace nabu {

// An utility to pass types as arguments.

template <class...Ts>
struct type_tag {};

template <class T>
struct type_tag<T> { using type = T; };

template <template <class...> class...>
struct hightype_tag {};

namespace detail {
    template <class T>
    struct tuple_tag {};

    template <class...Ts>
    struct tuple_tag<std::tuple<Ts...>> {
        using type = type_tag<Ts...>;
    };
}

template <class T>
using tuple_tag = typename detail::tuple_tag<T>::type;

// A variable always equal to false. Used for static_asserts.

template <class T>
constexpr bool always_false = false;

// is_detected indicates if an expression is valid or not.

namespace detail {
     template <template <class...> class Expression, class SFINAE, class...Ts>
     constexpr bool is_detected = false;
     
     template <template <class...> class Expression, class...Ts>
     constexpr bool is_detected<
          Expression,
          std::void_t<Expression<Ts...>>,
          Ts...
     > = true;
}

template <template <class...> class Expression, class...Ts>
constexpr bool is_detected = detail::is_detected<Expression, void, Ts...>;

// curry_left and curry_right pass arguments to a template class.

template <template <class...> class T, class...Args>
struct curry_left {
    template <class...News>
    using type = T<Args..., News...>;
};

template <template <class...> class T, class...Args>
struct curry_right {
    template <class...News>
    using type = T<News..., Args...>;
};

// Metafunctions applying a predicate (with static bool value) to all types passed.

template <template <class> class Predicate, class...Ts>
constexpr bool all_of(type_tag<Ts...>) {
    return (... && Predicate<Ts>::value);
}

template <template <class> class Predicate, class...Ts>
constexpr bool any_of(type_tag<Ts...>) {
    return (... || Predicate<Ts>::value);
}

// Collection traits.

// is_iterable check std::begin(t) and std::end(t) existence.

template <class T, class SFINAE = void>
constexpr bool is_iterable = false;

template <class T>
constexpr bool is_iterable<T, std::void_t<decltype(
    std::begin(std::declval<T&>()),
    std::begin(std::declval<T const&>()),
    std::end(std::declval<T&>()),
    std::end(std::declval<T const&>())
)>> = true;

template <class T>
using iterable_type = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

// is_continuous_iterable check std::data(t) and std::size(t) existence.

template <class T, class SFINAE = void>
constexpr bool is_continuous_iterable = false;

template <class T>
constexpr bool is_continuous_iterable<T, std::void_t<decltype(
    std::data(std::declval<T&>()),
    std::data(std::declval<T const&>()),
    std::size(std::declval<T const&>())
)>> = true;

template <class T>
using continuous_iterable_type = std::remove_reference_t<decltype(*std::data(std::declval<T&>()))>;

// reserve(t, nb) calls t.reserve(nb) if it exists.

namespace detail {
    template <class T>
    using reserve_expression = decltype(std::declval<T&>().reserve(std::declval<size_t>()));
}

template <class T>
void reserve(T& val, size_t nb) {
    if constexpr (is_detected<detail::reserve_expression, T>) {
        val.reserve(nb);
    }
}

// resize(t, nb) calls t.resize(nb).

namespace detail {
    template <class T>
    using resize_expression = decltype(std::declval<T&>().resize(std::declval<size_t>()));
}

template <class T>
constexpr bool has_resize = is_detected<detail::resize_expression, T>;

// emplace(t, val) emplaces val in t using emplace_back, emplace_front or emplace.

namespace detail {
    template <class T, class...Args>
    using emplace_back_expression = decltype(std::declval<T&>().emplace_back(std::forward<Args>(std::declval<Args>())...));
    template <class T, class...Args>
    using emplace_front_expression = decltype(std::declval<T&>().emplace_front(std::forward<Args>(std::declval<Args>())...));
    template <class T, class...Args>
    using emplace_expression = decltype(std::declval<T&>().emplace(std::forward<Args>(std::declval<Args>())...));
}

template <class T, class...Args>
constexpr bool has_emplace =
    is_detected<detail::emplace_back_expression,  T, Args...> ||
    is_detected<detail::emplace_front_expression, T, Args...> ||
    is_detected<detail::emplace_expression,       T, Args...>;

template <class T, class...Args>
decltype(auto) emplace(T& data, Args&&...args) {
    static_assert(has_emplace<T, Args...>);
    if constexpr (is_detected<detail::emplace_back_expression, T, Args...>) {
        return data.emplace_back(std::forward<Args>(args)...);
    }
    else if constexpr (is_detected<detail::emplace_front_expression, T, Args...>) {
        return data.emplace_front(std::forward<Args>(args)...);
    }
    else {
        return data.emplace(std::forward<Args>(args)...);
    }
}

// for_each(tuple, f) applies f to each member of the tuple.

template <class...Ts, class F>
void for_each(std::tuple<Ts...>& tuple, F&& f) {
    (..., f(std::get<Ts>(tuple)));
}

template <class...Ts, class F>
void for_each(std::tuple<Ts...> const& tuple, F&& f) {
    (..., f(std::get<Ts>(tuple)));
}

// Removes constness of values, references, pairs and tuples.

namespace detail {
    template <class T, class = void>
    struct remove_deep_const_t {
        using type = T;
    };
}

template <class T>
using remove_deep_const = typename detail::remove_deep_const_t<T>::type;

namespace detail {
    template <class T>
    struct remove_deep_const_t<T const, void> {
        using type = remove_deep_const<T>;
    };
    template <class T>
    struct remove_deep_const_t<T const&, void> {
        using type = remove_deep_const<T>&;
    };
    template <class T1, class T2>
    struct remove_deep_const_t<std::pair<T1, T2>, void> {
        using type = std::pair<remove_deep_const<T1>, remove_deep_const<T2>>;
    };
    template <class...Ts>
    struct remove_deep_const_t<std::tuple<Ts...>, void> {
        using type = std::tuple<remove_deep_const<Ts>...>;
    };
}

template <class T>
using mutable_iterable_type = remove_deep_const<iterable_type<T>>;

template <class T>
using mutable_continuous_iterable_type = remove_deep_const<continuous_iterable_type<T>>;

// first_arg_of<F> indicates the first argument of F.

template <class F>
constexpr bool takes_argument = std::tuple_size_v<boost::callable_traits::args_t<F>> != 0;

namespace detail {
	template <class F, bool = takes_argument<F>>
	struct first_arg_of_t {
		using type = void;
	};
	template <class F>
	struct first_arg_of_t<F, true> {
		using type = std::tuple_element_t<0, boost::callable_traits::args_t<F>>;
	};
}

template <class F>
using first_arg_of = detail::first_arg_of_t<F>::value;

// return_of<F> indicates the return argument of F.

template <class F>
using return_of = boost::callable_traits::return_type_t<F>;

} // nabu
