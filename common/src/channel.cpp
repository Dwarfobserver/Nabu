
#include <common/channel.hpp>
#include <common/messages.hpp>
#include <common/reactor_base.hpp>
#include <chrono>

#if defined(MERGE_CLIENT_SERVER)
#include "detail/channel_merge.inl"
#else
#include "detail/channel_split.inl"
#endif

std::future<channel> connect_with(reactor_base& reactor, person_id const& src, person_id const& dst) {
    using namespace std::chrono;
    using namespace std::literals;

    auto promise = std::promise<channel>{};
    auto future  = promise.get_future();
    auto c       = make_channel(src, dst);

    c << msg::connect_request{};

    reactor.add_recurrent([ // TODO Handle timeout
        src,
        dst,
        promise      = std::move(promise),
        c            = std::move(c),
        request_time = high_resolution_clock::now(),
        response     = msg::connect_response{}
    ]
    () mutable -> bool
    {
        if (c.is_empty()) {
            auto now = high_resolution_clock::now();

            if (now - request_time < 1000ms)
                return false;
            
            try {
                throw connection_error{"The channel connection has timeout"};
            } catch (...) {
                promise.set_exception(std::current_exception());
            }
            return true;
        }

        c >> response;

        if (response.accepted) {
            promise.set_value(make_channel(src, response.id));
            return true;
        }

        try {
            throw connection_error{"The channel connection has been refused"};
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
        return true;
    });
    return future;
}
