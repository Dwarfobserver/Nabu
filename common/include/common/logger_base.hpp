
#include <common/core.hpp>
#include <fstream>
#include <mutex>

enum class logger_verbosity {
    debug   = 0,
    status  = 1,
    warning = 2,
    error   = 3
};

inline char const* to_string(logger_verbosity v) noexcept {
    switch (v) {
        case logger_verbosity::debug:   return "debug";
        case logger_verbosity::status:  return "status";
        case logger_verbosity::warning: return "warning";
        case logger_verbosity::error:   return "error";
    }
    return "*invalid logger_verbosity value*";
}

template <logger_verbosity Verbosity>
class logger_base {

    template <logger_verbosity Level>
    class input {
        friend class logger_base<Verbosity>;

        static constexpr bool enabled =
            static_cast<int>(Level) >= static_cast<int>(Verbosity);
    public:
        template <class T>
        input const& operator<<(T const& data) const {
            if constexpr (enabled) {
                std::lock_guard l{ logger_.mutex_ };
                logger_.file_ << data;
            }
            return *this;
        }
    private:
        input(logger_base& logger) : logger_(logger) {}
        logger_base& logger_;
    };
    template <logger_verbosity>
    friend class input;
public:
    logger_base(char const* file) :
            verbosity(Verbosity),
            debug  (*this),
            status (*this),
            warning(*this),
            error  (*this),
            file_{ file }
    {}

    logger_verbosity const verbosity;
    input<logger_verbosity::debug>   const debug;
    input<logger_verbosity::status>  const status;
    input<logger_verbosity::warning> const warning;
    input<logger_verbosity::error>   const error;

    void flush() {
        file_.flush();
    }
private:
    std::ofstream file_;
    std::mutex mutex_;
};
