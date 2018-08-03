
#pragma once

#include <string>


/* TODO
#if defined(_MSC_VER)
   #define NABU_PUBLIC __declspec(dllexport) ...
#endif*/

// NABU_PP_CAT concatene arguments after expanding them.

#define NABU_PP_CAT2(a, b) a ## b
#define NABU_PP_CAT(a, b) NABU_PP_CAT2(a, b)

// NABU_PP_STR stringify argument after expanding it.

#define NABU_PP_STR2(a) # a
#define NABU_PP_STR(a) NABU_PP_STR2(a)

// Nifty Counter global variable declaration.
// TODO Accept comas in type with va_args

#define NABU_DECLARE_GLOBAL(type, name) \
    extern type& name; \
     \
    namespace detail { \
        static struct name##_nc_initializer_t { \
            name##_nc_initializer_t(); \
            ~name##_nc_initializer_t(); \
        } name##_nc_initializer{}; \
    } \
    struct name##_nc_force_semicolon

// Implements the global variable declared with NABU_DECLARE_GLOBAL. Must be in a source file.
// The arguments after 'name' are forwarded to the constructor.

#define NABU_IMPLEMENT_GLOBAL(name, ...) \
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

// Used to add a destructor to a lambda. The macro is used in it's capture list.

#define NABU_CAPTURE_DESTRUCTOR(...) \
    NABU_PP_CAT(_dtor_,__LINE__) = [] { \
        struct _nabu_capture_dtor_t { \
            bool empty; \
            _nabu_capture_dtor_t() noexcept : \
                empty(false) { } \
            ~_nabu_capture_dtor_t() noexcept(false) \
                { if (!empty) { __VA_ARGS__; } } \
            _nabu_capture_dtor_t(_nabu_capture_dtor_t&& rhs) noexcept : \
                empty(false) { rhs.empty = true; } \
        }; \
        return _nabu_capture_dtor_t{}; \
    } ()

// Correspond to "X.x" (X = NABU_VERSION_MAJOR, x = NABU_VERSION_MINOR).

#define NABU_VERSION_STRING NABU_PP_STR(NABU_PP_CAT(NABU_VERSION_MAJOR, NABU_PP_CAT(., NABU_VERSION_MINOR)))

// Define NABU_BUILD_TYPE_STRING equal to "Debug" or "Release".
// Check that Exactly one of _DEBUG or NDEBUG is defined.

#if (defined(_DEBUG) &&  defined(NDEBUG)) || \
   (!defined(_DEBUG) && !defined(NDEBUG))

    #error Exactly one of _DEBUG or NDEBUG must be defined.

#elif defined(_DEBUG)
    #define NABU_BUILD_TYPE_STRING "Debug"
#else
    #define NABU_BUILD_TYPE_STRING "Release"
#endif

// Only GCC, Clang and MSVC are supported.

#if !defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
    #error Compiler not supported
#endif

// likely(x) and unlikely(x) indicates that 'x' tends to be evaluated to respectly true and false.

#if defined(_MSC_VER)
    #define likely(x)   (static_cast<bool>(x))
    #define unlikely(x) (static_cast<bool>(x))
#else
    #define likely(x)   (__builtin_expect(static_cast<bool>(x), true))
    #define unlikely(x) (__builtin_expect(static_cast<bool>(x), false))
#endif

// force_inline tries to inline the given function (it can failed, especially in debug builds).
// never_inline will avoid to inline the function call.

#if defined(_MSC_VER)
    #define force_inline __forceinline
    #define never_inline __declspec(noinline)
#else
    #define force_inline inline [[gnu::always_inline]]
    #define never_inline [[gnu::noinline]]
#endif

// NABU_ASSERT(x, msg) allows optimisation in release assuming that 'x' evaluates to true,
// and checks the boolean in debug. If it's false, it will throw an exception.

#if !defined(_DEBUG)
    #if defined(_MSC_VER)
        #define NABU_ASSERT(x, msg) __assume(x)
    #else
        #define NABU_ASSERT(x, msg) static_cast<void>(0)
    #endif
#else
    #include <fmt/format.h>
    #include <stdexcept>
    
    namespace nabu {
        class assert_failed_exception : public std::logic_error {
        public:
            explicit assert_failed_exception(std::string const& msg) : std::logic_error{ msg } {}
        };
    }
    #define NABU_ASSERT(x, ...) static_cast<void>(likely(x) || \
                (throw ::nabu::assert_failed_exception{fmt::format( \
                    "At file {} :\n" \
                    "Assert failed in line {}, at '{}'.\n" \
                    "Message : '{}'." \
                __FILE__, static_cast<int>(__LINE__), #x, fmt::format(__VA_ARGS__)) }, true))
#endif

// unreachable() marks a code path as unreachable, allowing optimisations on release an failing
// assert on debug.

#if !defined(_DEBUG)
    #if defined(_MSC_VER)
        #define unreachable() __assume(false)
    #else
        #define unreachable() __builtin_unreachable()
    #endif
#else
    #define unreachable() NABU_ASSERT(false, "unreachable() macro reached")
#endif
