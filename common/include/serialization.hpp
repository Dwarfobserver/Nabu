
#pragma once

#include <serialization_base.hpp>
#include <optional>
#include <variant>


namespace nabu {

// Implementation for optional<T>.

template <class T>
constexpr int custom_serialized_size(std::optional<T> const& opt) noexcept {
    return sizeof(bool) + opt ? serialized_size(*opt) : 0;
}

template <class T>
int custom_serialized_size(buffer_span const& span, type_tag<std::optional<T>>) noexcept {
    if (span.size() < sizeof(bool))
        return invalid_serialized_size;

    auto s = span;
    bool has_val;
    deserialize(s, has_val);
    if (!has_val) return sizeof(bool);
    int const size = serialized_size(s, type_tag<T>{});

    if (size == invalid_serialized_size)
        return invalid_serialized_size;

    return sizeof(bool) + size;
}

template <class T>
void custom_serialize(buffer_span& span, std::optional<T> const& opt) {
    auto has_val = opt.has_value();
    serialize(span, has_val);
    if (has_val) serialize(span, *opt);
}

template <class T>
void custom_deserialize(buffer_span& span, std::optional<T>& opt) {
    bool has_val;
    deserialize(span, has_val);
    if (has_val) {
        T v;
        deserialize(span, v);
        opt = std::move(v);
    }
    else opt.reset();
}

// Implementation for std::pair<T1, T2>.

template <class T1, class T2>
constexpr int custom_serialized_size(std::pair<T1, T2> const& pair) noexcept {
    return serialized_size(pair.first) + serialized_size(pair.second);
}

template <class T1, class T2>
int custom_serialized_size(buffer_span const& span, type_tag<std::pair<T1, T2>>) noexcept {

    int const size_1 = serialized_size(span, type_tag<T1>{});
    if (size_1 == invalid_serialized_size)
        return invalid_serialized_size;
    
    auto s = span;
    s.begin += size_1;
    int const size_2 = serialized_size(s, type_tag<T2>{});
    if (size_2 == invalid_serialized_size)
        return invalid_serialized_size;

    return size_1 + size_2;
}

template <class T1, class T2>
void custom_serialize(buffer_span& span, std::pair<T1, T2> const& pair) {
    serialize(span, pair.first);
    serialize(span, pair.second);
}

template <class T1, class T2>
void custom_deserialize(buffer_span& span, std::pair<T1, T2>& pair) {
    deserialize(span, pair.first);
    deserialize(span, pair.second);
}

// Implementation for std::variant<Ts...>.
/*
template <class...Ts>
constexpr int custom_serialized_size(std::variant<Ts...> const& variant) noexcept {
    if (variant.valueless_by_exception()) throw std::bad_variant_access{};
    
}

template <class T1, class T2>
int custom_serialized_size(buffer_span const& span, type_tag<std::variant<Ts...>>) noexcept {
    if (variant.valueless_by_exception()) throw std::bad_variant_access{};
    
}

template <class T1, class T2>
void custom_serialize(buffer_span& span, std::variant<Ts...> const& variant) {
    if (variant.valueless_by_exception()) throw std::bad_variant_access{};
    
}

template <class T1, class T2>
void custom_deserialize(buffer_span& span, std::variant<Ts...>& variant) {
    if (variant.valueless_by_exception()) throw std::bad_variant_access{};
    
}
*/
} // nabu
