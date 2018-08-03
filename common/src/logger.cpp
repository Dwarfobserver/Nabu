
#include <logger.hpp>
#include <iostream>
#include <fstream>


namespace nabu {

namespace {

class console_stream : public logger_stream_interface {
public:
    logger_stream_interface& operator<<(std::string const& msg) override
        { return std::cout << msg, *this; }
};

class file_stream : public logger_stream_interface {
    std::ofstream file_;
public:
    file_stream(std::string const& file) : file_{ file } {}

    logger_stream_interface& operator<<(std::string const& msg) override
        { return file_ << msg, *this; }
};

class tests_stream : public logger_stream_interface {
public:
    logger_stream_interface& operator<<(std::string const& msg) override
        { return *this; }
};

} // <anonymous>

#if defined(BUILDING_TESTS)
constexpr bool nabu_building_tests = true;
#else
constexpr bool nabu_building_tests = false;
#endif


NABU_IMPLEMENT_GLOBAL(logger,
    [] {
        auto& v = configuration.logger.verbosity;
        if (v == "debug")    return logger_level::debug;
        if (v == "info")     return logger_level::info;
        if (v == "warning")  return logger_level::warning;
        if (v == "error")    return logger_level::error;
        if (v == "disabled") return logger_level::disabled;
        std::cout << "Invalid logger.verbosity value : it must be one of 'debug', 'info', 'warning', 'error' or 'disabled'.\n";
        std::terminate();
    } (),
    [NABU_CAPTURE_DESTRUCTOR(logger.debug("logger created"))] () -> std::unique_ptr<logger_stream_interface> {
        if (nabu_building_tests)
            return std::make_unique<tests_stream>();

        auto& out = configuration.logger.output;
        if (out == "console")
             return std::make_unique<console_stream>();
        else return std::make_unique<file_stream>(out);
    } ()
);

} // nabu
