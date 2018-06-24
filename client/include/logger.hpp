
#include <common/logger_base.hpp>

namespace detail {
    inline logger_base<logger_verbosity::CLIENT_LOGGER_VERBOSITY> client_logger{"client_logs.txt"};
}

static auto& logger = detail::client_logger;
