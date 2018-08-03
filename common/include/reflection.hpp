
#pragma once

#include <limits>
#include <map>


namespace nabu {

// Demangle the given type name.

std::string demangle(char const* name);

namespace detail {
    template <class T>
    inline std::string const name_of = demangle(typeid(T).name());
}

template <class T>
std::string name_of() {
    return detail::name_of<T>;
}

// For polymorphic types.
template <class T>
std::string name_of(T const& val) {
    return demangle(typeid(val).name());
}

// Reflection utilities for classes having an id.
// They expose it with id_type const T::id.

using id_type = int8_t;

constexpr id_type id_min_value = 0;
constexpr id_type id_max_value = 50;

constexpr bool is_valid_id(id_type id) {
    return id >= id_min_value && id <= id_max_value;
}

template <class T, class SFINAE = void>
constexpr bool has_id = false;

template <class T>
constexpr bool has_id<T, std::enable_if_t<
    std::is_integral_v<decltype(T::id)>
>> = true;

template <class T>
constexpr id_type id_of() {
    static_assert(has_id<T>);
    static_assert(is_valid_id(T::id));
    return T::id;
};

} // nabu
