
#pragma once

#include <aggregates_to_tuples.hpp>
#include <type_traits>
#include <cstring>

// is_iterable indicates if the type is iterable with 'std::begin(T)' and 'std::end(T)'.
// It also verify that an element can be added with 'T.insert(T.end(), T::value_type&&)'.

// TODO Specialized method to insert range for everyone.

template <class T, class SFINAE = void>
constexpr bool is_iterable = false;

template <class T>
constexpr bool is_iterable<T, std::void_t<decltype(
    std::begin(std::declval<T&>()),
    std::begin(std::declval<T const&>()),
    std::end(std::declval<T&>()),
    std::end(std::declval<T const&>()),
    std::distance(
        std::begin(std::declval<T const&>()),
        std::end(std::declval<T const&>())
    ),
    std::declval<T&>().insert(
        std::end(std::declval<T const&>()),
        std::declval<typename T::value_type&&>()
    )
)>> = true;

// is_continuous_iterable indicates if a type stores it's elements continuously in memory,
// with 'size()' and 'data()'.

template <class T, class SFINAE = void>
constexpr bool is_continuous_iterable = false;

template <class T>
constexpr bool is_continuous_iterable<T, std::enable_if_t<
    is_iterable<T> &&
    std::is_pointer_v <decltype(std::declval<T&>().data())> &&
    std::is_pointer_v <decltype(std::declval<T const&>().data())> &&
    std::is_integral_v<decltype(std::declval<T const&>().size())>
>> = true;

namespace detail {
    template <class T>
    using data_value_type = std::remove_pointer_t<decltype(std::declval<T&>().data())>;
}

// is_continuous_trivially_iterable indicates if the objects stored by T can be all copied
// in one time (because they are trivially copyable).

template <class T, class SFINAE = void>
constexpr bool is_continuous_trivially_iterable = false;

template <class T>
constexpr bool is_continuous_trivially_iterable<T, std::enable_if_t<
    is_continuous_iterable<T> &&
    std::is_trivially_copyable_v<detail::data_value_type<T>>
>> = true;

// has_custom_serialization indicates if the object has these functions defined for him :
//
//   - custom_serialized_size(T const& data)
//   - custom_serialize(T const& data, std::byte* dst)
//   - custom_deserialize(T& data, std::byte const* src)
//
// custom_serialize and custom_deserialize must return custom_serialized_size, which corresponds
// to the number of bytes written to dst or read from src.

template <class T, class SFINAE = void>
constexpr bool has_custom_serialization = false;

template <class T>
constexpr bool has_custom_serialization<T, std::enable_if_t<
    std::is_integral_v<decltype(custom_serialized_size(std::declval<T const&>()))> &&
    std::is_integral_v<decltype(custom_serialize      (std::declval<T const&>(), std::declval<std::byte*>()))> &&
    std::is_integral_v<decltype(custom_deserialize    (std::declval<T&>(),       std::declval<std::byte const*>()))>
>> = true;

using iterable_serialized_size_type = uint16_t;

// Serialization actions try to serialize objects regarding their properties in this order :
//
//  - 1) has the object custom serialization
//  - 2) is the object trivially copyable
//  - 3) is the object continuously trivially iterable
//  - 4) is the object iterable
//  - 5) is the object an aggregate
//
// Else, emits a compile-time error.

namespace detail {

    // Used to trick static_assert to fail only when it's branch is taken.

    template <class T>
    struct always_false_t { enum { value = 0 }; };

    template <class T>
    constexpr bool always_false = always_false_t<T>::value;
}

// Categorizes the class regarding how she will be serialized.

namespace serial {
    struct custom {};
    struct trivial {};
    struct continuous_trivial {};
    struct iterable {};
    struct aggregate {};
    struct none {};
}

namespace detail {
    template <class T>
    constexpr auto get_serialization_tag() {
        if constexpr (has_custom_serialization<T>) {
            return serial::custom{};
        }
        else if constexpr (std::is_trivially_copyable_v<T>) {
            return serial::trivial{};
        }
        else if constexpr (is_continuous_trivially_iterable<T>) {
            return serial::continuous_trivial{};
        } 
        else if constexpr (is_iterable<T>) {
            return serial::iterable{};
        }
        else if constexpr (att::is_aggregate<T>) {
            return serial::aggregate{};
        }
        else {
            return serial::none{};
        }
    }
}

template <class T>
using serialization_tag = decltype(detail::get_serialization_tag<T>());

template <class T>
constexpr bool is_serializable = !std::is_same_v<
    serialization_tag<T>,
    serial::none
>;

// Dispatch calls using serialization_tag.

template <class T>
constexpr int serialized_size(T const& val);

template <class T>
int serialize(T const& val, std::byte* dst);

template <class T>
int deserialize(T& val, std::byte const* src);

namespace detail {

    // Custom

    template <class T>
    int serialized_size(T const& val, serial::custom) {
        return custom_serialized_size(val);
    }

    template <class T>
    int serialize(T const& val, std::byte* dst, serial::custom) {
        return custom_serialize(val, dst);
    }

    template <class T>
    int deserialize(T& val, std::byte const* src, serial::custom) {
        return custom_deserialize(val, src);
    }

    // Trivial

    template <class T>
    constexpr int serialized_size(T const& val, serial::trivial) {
        return sizeof(T);
    }

    template <class T>
    int serialize(T const& val, std::byte* dst, serial::trivial) {
        int const size = ::serialized_size(val);
        std::memcpy(dst, &val, size);
        return size;
    }

    template <class T>
    int deserialize(T& val, std::byte const* src, serial::trivial) {
        int const size = ::serialized_size(val);
        std::memcpy(&val, src, size);
        return size;
    }

    // Continuous trivial

    template <class T>
    int serialized_size(T const& val, serial::continuous_trivial) {
        return sizeof(iterable_serialized_size_type) + val.size() * sizeof(detail::data_value_type<T>);
    }

    template <class T>
    int serialize(T const& val, std::byte* dst, serial::continuous_trivial) {
        iterable_serialized_size_type const nb = val.size(); // TODO checck size
        int const size = ::serialized_size(val);

        dst += ::serialize(nb, dst);
        std::memcpy(dst, val.data(), size - ::serialized_size(nb));
        return size;
    }

    template <class T>
    int deserialize(T& val, std::byte const* src, serial::continuous_trivial) {
        using value_t = detail::data_value_type<T>;
        iterable_serialized_size_type nb;

        src += ::deserialize(nb, src);
        auto const ptr = reinterpret_cast<value_t const*>(src);
        val.assign(ptr, ptr + nb);
        return ::serialized_size(val);
    }

    // Iterable

    template <class T>
    int serialized_size(T const& val, serial::iterable) {
        int sum = sizeof(iterable_serialized_size_type);
        for (auto const& v : val) {
            sum += ::serialized_size(v);
        }
        return sum;
    }

    template <class T>
    int serialize(T const& val, std::byte* dst, serial::iterable) {
        const auto dst_beg = dst;
        iterable_serialized_size_type nb = 0;

        dst += sizeof(nb);
        for (auto const& v : val) {
            dst += ::serialize(v, dst);
            ++nb;
        }
        ::serialize(nb, dst_beg);
        return std::distance(dst_beg, dst);
    }

    template <class T>
    int deserialize(T& val, std::byte const* src, serial::iterable) {
        using value_t = typename T::value_type;
        const auto src_beg = src;
        iterable_serialized_size_type nb;

        ::deserialize(nb, src_beg);
        src += sizeof(nb);

        val.clear();
        value_t v;
        for (int i = 0; i < nb; ++i) {
            src += ::deserialize(v, src);
            val.insert(std::end(val), std::move(v));
        }
        return std::distance(src_beg, src);
    }

    // Aggregate

    template <class T>
    int serialized_size(T const& val, serial::aggregate) {
        int sum = 0;
        att::for_each(val, [&sum] (auto const& v) {
            sum += ::serialized_size(v);
        });
        return sum;
    }
    template <class T>
    int serialize(T const& val, std::byte* dst, serial::aggregate) {
        auto const dst_beg = dst;
        att::for_each(val, [&dst] (auto const& v) {
            dst += ::serialize(v, dst);
        });
        return std::distance(dst_beg, dst);
    }
    template <class T>
    int deserialize(T& val, std::byte const* src, serial::aggregate) {
        const auto src_beg = src;
        att::for_each(val, [&src] (auto& v) {
            src += ::deserialize(v, src);
        });
        return std::distance(src_beg, src);
    }

}

// serialized_size(val) returns the number of bytes taken to represent the object.

template <class T>
constexpr int serialized_size(T const& val) {
    static_assert(is_serializable<T>);
    return detail::serialized_size(val, serialization_tag<T>{});
}

// serialize(val, dst) writes the object to dst, and return it's serialized size.

template <class T>
int serialize(T const& val, std::byte* dst) {
    static_assert(is_serializable<T>);
    return detail::serialize(val, dst, serialization_tag<T>{});
}

// deserialize(val, dst) reads the object from src, and return it's serialized size.

template <class T>
int deserialize(T& val, std::byte const* src) {
    static_assert(is_serializable<T>);
    return detail::deserialize(val, src, serialization_tag<T>{});
}

