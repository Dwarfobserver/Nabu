
#pragma once

#include <type_traits>

// PP_CAT concatene arguments after expanding them.

#define PP_CAT2(a, b) a ## b
#define PP_CAT(a, b) PP_CAT2(a, b)

// PP_STR stringify argument after expanding it.

#define PP_STR2(a) # a
#define PP_STR(a) PP_STR2(a)

// Nifty Counter global variable declaration.

#define DECLARE_NC_GLOBAL(type, name) \
    extern type& name; \
     \
    namespace detail { \
        static struct name##_nc_initializer_t { \
            name##_nc_initializer_t(); \
            ~name##_nc_initializer_t(); \
        } name##_nc_initializer{}; \
    } \
    struct name##_nc_force_semicolon

// Implements the global variable declared with DECLARE_NC_GLOBAL. Must be in a source file.
// The arguments after 'name' are forwarded to the constructor.

#define IMPLEMENT_NC_GLOBAL(name, ...) \
    namespace { \
        using name##_nc_type = std::remove_reference_t<decltype(name)>; \
        int name##_nifty_counter; \
         \
        std::aligned_storage_t< \
            sizeof(name##_nc_type), \
            alignof(name##_nc_type) \
        > name##_nc_storage; \
    } \
     \
    name##_nc_type & name = reinterpret_cast<name##_nc_type &>(name##_nc_storage); \
     \
    namespace detail { \
        name##_nc_initializer_t::name##_nc_initializer_t() { \
            if (name##_nifty_counter++ == 0) { \
                new (&name##_nc_storage) name##_nc_type{ __VA_ARGS__ }; \
            } \
        } \
        name##_nc_initializer_t::~name##_nc_initializer_t() { \
            if (--name##_nifty_counter == 0) { \
                name.~name##_nc_type(); \
            } \
        } \
    } \
    struct name##_nc_force_semicolon




