
#pragma once

#include <meta.hpp>
#include <environment.hpp>
#include <reflection.hpp>
#include <aggregates_to_tuples.hpp>
#include <fmt/format.h>
#include <limits>
#include <cstring>
#include <iterator>


namespace nabu {

// Span inherited by stream classes for serialization.

struct buffer_span {
    std::byte* begin;
    std::byte* end;

    int  size()     const noexcept { return end - begin; }
    bool is_empty() const noexcept { return size() == 0; }
};

namespace stream_policy {
    struct raw {};
    struct throwing {};
    struct checked {};
}

namespace detail {
    template <class StreamPolicy>
    struct stream_mixin {};

    template <>
    struct stream_mixin<stream_policy::checked> {
        bool error = false;
    };
}

template <class StreamPolicy>
struct basic_stream : buffer_span, detail::stream_mixin<StreamPolicy> {
    using policy_type = StreamPolicy;

    basic_stream() noexcept :
        buffer_span{ nullptr, nullptr } {}
    
    basic_stream(std::byte* const begin, std::byte* const end) noexcept :
        buffer_span{ begin, end } {}
        
    basic_stream(std::byte* const data, int const size) noexcept :
        buffer_span{ data, data + size } {}
    
    template <class T, class = std::enable_if_t<
        is_continuous_iterable<T> &&
        std::is_same_v<
            continuous_iterable_type<T>,
            std::byte
        >
    >>
    basic_stream(T& array) noexcept :
        buffer_span{ std::data(array), std::data(array) + std::size(array) } {}
};

using raw_stream     = basic_stream<stream_policy::raw>;
using checked_stream = basic_stream<stream_policy::checked>;
using throw_stream   = basic_stream<stream_policy::throwing>;

// raw_stream operations are not checked : the buffer can be overflowed.

template <class T>
raw_stream& operator<<(raw_stream& stream, T const& val) {
    serialize(stream, val);
    return stream;
}
template <class T>
raw_stream& operator>>(raw_stream& stream, T& val) {
    deserialize(stream, val);
    return stream;
}

// checked_stream keep it's valid state with the boolean 'error'.
// Once it could not stream objets, it will refuse any operation.

template <class T>
checked_stream& operator<<(checked_stream& stream, T const& val) {
    if (stream.error) return stream;
    stream.error = !try_serialize(stream, val);
    return stream;
}
template <class T>
checked_stream& operator>>(checked_stream& stream, T& val) {
    if (stream.error) return stream;
    stream.error = !try_deserialize(stream, val);
    return stream;
}

// throw_stream will throw exceptions on failure.

template <class T>
throw_stream& operator<<(throw_stream& stream, T const& val) {
    throw_serialize(stream, val);
    return stream;
}
template <class T>
throw_stream& operator>>(throw_stream& stream, T& val) {
    throw_deserialize(stream, val);
    return stream;
}

// Exceptions thrown from overflows.
// Writes are considered bugs (we know the data sent and the buffer),
// but not reads (because we don't know the data received).

class write_buffer_overflow : public std::logic_error {
public:
    inline explicit write_buffer_overflow(std::string const& str) :
        std::logic_error{ str } {}
};

class read_buffer_overflow : public std::runtime_error {
public:
    inline explicit read_buffer_overflow(std::string const& str) :
        std::runtime_error{ str } {}
};

// The type serialized to store the collections items count.
// TODO serial_traits<T>::count_type.

using serialized_count_t = uint16_t;
constexpr size_t max_serialized_count = std::numeric_limits<serialized_count_t>::max();

#define NABU_ASSERT_SERIALIZED_COUNT(count) \
    NABU_ASSERT(count <= ::nabu::max_serialized_count, \
        "Can't serialize the count '{}' in the type '{}' (max value = {})", \
        count, ::nabu::name_of<serialized_count_t>(), max_serialized_count)

// Indicates if a type has custom serialization functions.
// The user must define the tested functions below.

template <class T, class SFINAE = void>
constexpr bool is_custom_serializable = false;

template <class T>
constexpr bool is_custom_serializable<T, std::void_t<decltype(
    custom_serialized_size(std::declval<T const&>()),
    custom_serialized_size(std::declval<buffer_span const&>(), std::declval<type_tag<T>>()),
    custom_serialize  (std::declval<buffer_span&>(), std::declval<T const&>()),
    custom_deserialize(std::declval<buffer_span&>(), std::declval<T&>())
)>> = true;

// The serial tag of a class is one of the following :
//
//   - serial_tag::custom :    nabu::is_custom_serializable<T> is true
//   - serial_tag::trivial :   std::is_trivially_copyable_v<T> is true
//   - serial_tag::array :     nabu::is_continuous_iterable<T> is true, and T holds trivial types.
//   - serial_tag::iterable :  nabu::is_iterable<T> is true, and T holds serializable types.
//   - serial_tag::aggregate : att::is_aggregate<T> is true, annd T is composed of serializable types.
//   - serial_tag::invalid :   none of the above.
//
// If a type has the properties of 2+ tags, the first listed is chosen.

namespace serial_tag {
    struct custom {};
    struct trivial {};
    struct array {};
    struct iterable {};
    struct aggregate {};
    struct invalid {};
}

namespace detail {
    template <class T> struct is_serializable;
    
    template <class T>
    constexpr auto serial_tag();
}

// Indicates the serial tag of a class.

template <class T>
using serial_tag_of = decltype(detail::serial_tag<remove_deep_const<std::remove_reference_t<T>>>());

namespace detail {

    template <class T>
    constexpr auto serial_tag_after_iterable()
    {
        if constexpr (att::is_aggregate<T>)
        {
               return serial_tag::aggregate{};
        }
        else { return serial_tag::invalid{}; }
    }

    template <class T>
    constexpr auto serial_tag_after_array()
    {
        if constexpr (is_iterable<T> && has_emplace<T>)
        {
            if constexpr (!std::is_same_v<
                            ::nabu::serial_tag_of<mutable_iterable_type<T>>,
                            serial_tag::invalid
            >) {
                   return serial_tag::iterable{};
            }
            else { return serial_tag_after_iterable<T>(); }
        }
        else     { return serial_tag_after_iterable<T>(); }
    }

    template <class T>
    constexpr auto serial_tag()
    {
        if constexpr (std::is_pointer_v<T>)
        {
            return serial_tag::invalid{};
        }
        else if constexpr (is_custom_serializable<T>)
        {
            return serial_tag::custom{};
        }
        else if constexpr (std::is_trivially_copyable_v<T>)
        {
            return serial_tag::trivial{};
        }
        else if constexpr (is_continuous_iterable<T> &&
                          (has_emplace<T> || has_resize<T>))
        {
            if constexpr (std::is_same_v<
                               ::nabu::serial_tag_of<mutable_continuous_iterable_type<T>>,
                               serial_tag::trivial
         >) {
                   return serial_tag::array{};
            }
            else { return serial_tag_after_array<T>(); }
        }
        else     { return serial_tag_after_array<T>(); }
    }
}

namespace detail {
    template <class T>
    struct is_serializable {
        static constexpr bool value = !std::is_same_v
            <serial_tag_of<T>, serial_tag::invalid>;
    };
}

// is_serializable<T> indicates that serial_tag_of<T> is not serial_tag::invalid.

template <class T>
constexpr bool is_serializable = detail::is_serializable<T>::value;

// Returns the serialized size in bytes of the given object.

template <class T>
constexpr int serialized_size(T const& val) noexcept {
    return detail::serialized_size(val, serial_tag_of<T>{});
}

// Returns the serialized size of the 'T' stored in span.
// If the span cannot contain the object, it returns 'invalid_serialized_size'.

constexpr int invalid_serialized_size = -1;

template <class T>
int serialized_size(buffer_span const& span, type_tag<T> tag) noexcept {
    return detail::serialized_size(span, tag, serial_tag_of<T>{});
}

// Serialize 'val' into 'span', advancing 'span.begin'.
// On failure :
//   - 'serialize' yields undefined behavior (due to buffer overflow).
//   - 'try_serialize' returns false.
//   - 'throw_serialize' throws a write_buffer_overflow (std::logic_error).

template <class T>
void serialize(buffer_span& span, T const& val) {
    detail::serialize(span, val, serial_tag_of<T>{});
}
template <class T>
bool try_serialize(buffer_span& span, T const& val) {
    int const size = serialized_size(val);
    if (span.size() < size) return false;
    serialize(span, val);
    return true;
}
template <class T>
void throw_serialize(buffer_span& span, T const& val) {
    if (!try_serialize(span, val)) throw write_buffer_overflow{ fmt::format(
        "Tried to serialize an object of type '{}' of size {} in a too small span of size {}",
        name_of<T>(), serialized_size(val), span.size() )};
}

// Deserialize 'val' from 'span', advancing 'span.begin'.
// On failure :
//   - 'deserialize' yields undefined behavior (due to buffer overflow).
//   - 'try_deserialize' returns false.
//   - 'throw_deserialize' throws a read_buffer_overflow (std::runtime_error).

template <class T>
void deserialize(buffer_span& span, T& val) {
    detail::deserialize(span, val, serial_tag_of<T>{});
}
template <class T>
bool try_deserialize(buffer_span& span, T& val) {
    int const size = serialized_size(span, type_tag<T>{});
    if (size == invalid_serialized_size) return false;
    deserialize(span, val);
    return true;
}
template <class T>
void throw_deserialize(buffer_span& span, T& val) {
    if (!try_deserialize(span, val)) throw read_buffer_overflow{ fmt::format(
        "Tried to deserialize an object of type '{}' of size {} in a too small span of size {}",
        name_of<T>(), serialized_size(span, type_tag<T>{}), span.size() )};
}

// Asserts for invalid types.

namespace detail {
    template <class T>
    constexpr int serialized_size(T const& val, serial_tag::invalid) noexcept {
        static_assert(always_false<T>, "[T] is not serializable");
    }
    template <class T>
    int serialized_size(buffer_span const& span, type_tag<T> tag, serial_tag::invalid) noexcept {
        static_assert(always_false<T>, "[T] is not serializable");
    }
    template <class T>
    void serialize(buffer_span& span, T const& val, serial_tag::invalid) {
        static_assert(always_false<T>, "[T] is not serializable");
    }
    template <class T>
    void deserialize(buffer_span& span, T& val, serial_tag::invalid) {
        static_assert(always_false<T>, "[T] is not serializable");
    }
}

// Indirection for custom types.

namespace detail {
    template <class T>
    constexpr int serialized_size(T const& val, serial_tag::custom) noexcept {
        return custom_serialized_size(val);
    }
    template <class T>
    int serialized_size(buffer_span const& span, type_tag<T> tag, serial_tag::custom) noexcept {
        return custom_serialized_size(span, tag);
    }
    template <class T>
    void serialize(buffer_span& span, T const& val, serial_tag::custom) {
        custom_serialize(span, val);
    }
    template <class T>
    void deserialize(buffer_span& span, T& val, serial_tag::custom) {
        custom_deserialize(span, val);
    }
}

// Implementation for trivial types.

namespace detail {
    template <class T>
    constexpr int serialized_size(T const& val, serial_tag::trivial) noexcept {
        return sizeof(T);
    }
    template <class T>
    int serialized_size(buffer_span const& span, type_tag<T>, serial_tag::trivial) noexcept {
        return span.size() >= sizeof(T) ? sizeof(T) : invalid_serialized_size;
    }
    template <class T>
    void serialize(buffer_span& span, T const& val, serial_tag::trivial) {
        memcpy(span.begin, &val, sizeof(T));
        span.begin += sizeof(T);
    }
    template <class T>
    void deserialize(buffer_span& span, T& val, serial_tag::trivial) {
        memcpy(&val, span.begin, sizeof(T));
        span.begin += sizeof(T);
    }
}

// Implementation for array types.

namespace detail {
    template <class T>
    constexpr int serialized_size(T const& val, serial_tag::array) noexcept {
        return sizeof(serialized_count_t) + sizeof(continuous_iterable_type<T>) * val.size();
    }
    template <class T>
    int serialized_size(buffer_span const& span, type_tag<T>, serial_tag::array) noexcept {
        if (span.size() < sizeof(serialized_count_t))
            return invalid_serialized_size;
        
        auto s = span;
        serialized_count_t count;
        ::nabu::deserialize(s, count);
        int const size = sizeof(count) + count * sizeof(continuous_iterable_type<T>);
        return span.size() >= size ? size : invalid_serialized_size;
    }
    template <class T>
    void serialize(buffer_span& span, T const& val, serial_tag::array) {
        NABU_ASSERT_SERIALIZED_COUNT(val.size());
        auto const count = static_cast<serialized_count_t>(val.size());
        ::nabu::serialize(span, count);

        auto const size = count * sizeof(continuous_iterable_type<T>);
        memcpy(span.begin, val.data(), size);
        span.begin += size;
    }
    template <class T>
    void deserialize(buffer_span& span, T& val, serial_tag::array) {
        serialized_count_t count;
        ::nabu::deserialize(span, count);

        T array;
        if constexpr (has_resize<T>) {
            array.resize(count);
            auto const size = count * sizeof(continuous_iterable_type<T>);
            memcpy(array.data(), span.begin, size);
            span.begin += size;
        }
        else {
            reserve(array, count);
            mutable_continuous_iterable_type<T> v;
            for (int i = 0; i < count; ++i) {
                ::nabu::deserialize(span, v);
                emplace(array, v);
            }
        }
        val = std::move(array);
    }
}

// Implementation for iterable types.

namespace detail {
    template <class T>
    constexpr int serialized_size(T const& val, serial_tag::iterable) noexcept {
        int sum = sizeof(serialized_count_t);
        for (auto const& v : val) sum += ::nabu::serialized_size(v);
        return sum;
    }
    template <class T>
    int serialized_size(buffer_span const& span, type_tag<T>, serial_tag::iterable) noexcept {
        if (span.size() < sizeof(serialized_count_t))
            return invalid_serialized_size;
        
        auto s = span;
        serialized_count_t count;
        ::nabu::deserialize(s, count);
        for (int i = 0; i < count; ++i) {
            int const size = ::nabu::serialized_size(s, type_tag<iterable_type<T>>{});
            if (size == invalid_serialized_size) return invalid_serialized_size;
            s.begin += size;
        }
        return s.begin - span.begin;
    }
    template <class T>
    void serialize(buffer_span& span, T const& val, serial_tag::iterable) {
        NABU_ASSERT_SERIALIZED_COUNT(val.size());
        auto const count = static_cast<serialized_count_t>(val.size());
        ::nabu::serialize(span, count);
        for (auto const& v : val) ::nabu::serialize(span, v);
    }
    template <class T>
    void deserialize(buffer_span& span, T& val, serial_tag::iterable) {
        serialized_count_t count;
        ::nabu::deserialize(span, count);

        T iterable;
        reserve(iterable, count);
        mutable_iterable_type<T> v;
        for (int i = 0; i < count; ++i) {
            ::nabu::deserialize(span, v);
            emplace(iterable, std::move(v));
        }
        val = std::move(iterable);
    }
}

// Implementation for tuples.

template <class...Ts>
constexpr int custom_serialized_size(std::tuple<Ts...> const& tuple) noexcept {
    return (0 + ... + serialized_size(std::get<Ts>(tuple)));
}

constexpr int custom_serialized_size(buffer_span const& span, type_tag<std::tuple<>>) noexcept {
    return 0;
}
template <class T, class...Ts>
constexpr int custom_serialized_size(buffer_span const& span, type_tag<std::tuple<T, Ts...>>) noexcept {
    int const size = serialized_size(span, type_tag<T>{});
    if (size == invalid_serialized_size) return invalid_serialized_size;

    auto s = span;
    s.begin += size;
    int const rec = serialized_size(s, type_tag<std::tuple<Ts...>>{});
    if (rec == invalid_serialized_size) return invalid_serialized_size;

    return size + rec;
}

template <class...Ts>
void custom_serialize(buffer_span& span, std::tuple<Ts...> const& tuple) {
    for_each(tuple, [&] (auto const& v) {
        serialize(span, v);
    });
}

template <class...Ts>
void custom_deserialize(buffer_span& span, std::tuple<Ts...>& tuple) {
    for_each(tuple, [&] (auto& v) {
        deserialize(span, v);
    });
}

// Implementation for aggregates (same as tuples).

namespace detail {
    template <class T>
    constexpr int serialized_size(T const& val, serial_tag::aggregate) noexcept {
        return ::nabu::serialized_size(att::as_tuple(val));
    }
    template <class T>
    int serialized_size(buffer_span const& span, type_tag<T>, serial_tag::aggregate) noexcept {
        using tag_t = type_tag<att::as_tuple_t<T>>;
        return ::nabu::serialized_size(span, tag_t{});
    }
    template <class T>
    void serialize(buffer_span& span, T const& val, serial_tag::aggregate) {
        ::nabu::serialize(span, att::as_tuple(val));
    }
    template <class T>
    void deserialize(buffer_span& span, T& val, serial_tag::aggregate) {
        ::nabu::deserialize(span, att::as_tuple(val));
    }
}

} // nabu
