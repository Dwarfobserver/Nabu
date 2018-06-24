
#include <common/logger_base.hpp>

namespace detail {
    inline logger_base<logger_verbosity::SERVER_LOGGER_VERBOSITY> server_logger{"server_logs.txt"};
}

static auto& logger = detail::server_logger;
