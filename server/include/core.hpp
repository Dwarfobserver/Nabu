
#pragma once

#include <common/logger_base.hpp>
#include <common/reactor_base.hpp>
#include <common/channel.hpp>

namespace detail {
    inline logger_base<logger_verbosity::SERVER_LOGGER_VERBOSITY> server_logger{"server_logs.txt"};
    inline reactor_base server_reactor;
    inline person_id server_local_id = server_id;
}

static auto& logger = detail::server_logger;
static auto& reactor = detail::server_reactor;
static auto& local_id = detail::server_local_id;

inline channel make_channel(person_id const& dst) {
    return make_channel(local_id, dst);
}

inline std::future<channel> connect_with(person_id const& dst) {
    return connect_with(reactor, local_id, dst);
}
