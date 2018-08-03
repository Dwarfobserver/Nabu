
#include <reflection.hpp>
#include <logger.hpp>


namespace {
    std::string remove_struct_class_qualifier(std::string const& str) {
        auto view = std::string_view{ str }.substr(0, 6);
        if (view == "struct " || view == "class ") {
            return str.substr(6);
        }
        else return str;
    }
}

#if __has_include(<cxxabi.h>)

#include <cxxabi.h>

namespace nabu {
    std::string demangle(char const* const name) {
        using namespace std::literals;

        int status = 1;
        auto const ptr = abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status);
        auto const str = std::string{ ptr };
        ASSERT(status == 0, "abi::__cxa_demangle failed to demangle "s + name);
        std::free(ptr);
        return remove_struct_class_qualifier(str);
    }
}

#else

namespace nabu {
    std::string nabu::demangle(char const* const name) {
        return remove_struct_class_qualifier(name);
    }
}

#endif
