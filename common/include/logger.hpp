
#pragma once

#include <configuration.hpp> // 'logger' depends on 'configuration'.
#include <fmt/format.h>
#include <iostream> // 'logger' depends on 'std::cout'.
#include <memory>
#include <mutex>


namespace nabu {

namespace logger_level {
    enum {
        debug = 0,
        info,
        warning,
        error,
        disabled
    };
}

class logger_stream_interface {
public:
    virtual ~logger_stream_interface() = default;
    virtual logger_stream_interface& operator<<(std::string const& msg) = 0;
};

class logger_type {
    std::unique_ptr<logger_stream_interface> stream_;
    std::mutex stream_mutex_;

    template <int Level>
    class output {
        friend class logger_type;

        logger_type& log_;
        std::string header_;

        output(logger_type& log, std::string header) noexcept :
            log_{ log },
            header_{ std::move(header) }
        {}
    public:
        template <class String, class...Args>
        void operator()(String const& str, Args const&...args) const {
            if (Level < log_.level) return;
            std::lock_guard<std::mutex> g{ log_.stream_mutex_ };
            (*log_.stream_) << header_ << fmt::format(str, args...) << "\n";
        }
    };
public:
    int level;
    output<logger_level::debug>   const debug;
    output<logger_level::info>    const info;
    output<logger_level::warning> const warning;
    output<logger_level::error>   const error;

    inline logger_type(int level, std::unique_ptr<logger_stream_interface>&& stream) :
        stream_{ std::move(stream) },
        level  { level },
        debug  { *this, "[Debug]   " },
        info   { *this, "[Info]    " },
        warning{ *this, "[Warning] " },
        error  { *this, "[Error]   " }
    {}
};

NABU_DECLARE_GLOBAL(logger_type, logger);

} // nabu
