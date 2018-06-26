
#pragma once

#include <common/logger_base.hpp>
#include <common/reactor_base.hpp>
#include <common/channel.hpp>

namespace detail {
    inline logger_base<logger_verbosity::CLIENT_LOGGER_VERBOSITY> client_logger{ "client_logs.txt" };
    inline reactor_base client_reactor;
    inline person_id client_local_id{ "client" };
}

static auto& logger = detail::client_logger;
static auto& reactor = detail::client_reactor;
static auto& local_id = detail::client_local_id;

inline std::future<channel> connect_with(person_id const& dst) {
    return connect_with(reactor, local_id, dst);
}
