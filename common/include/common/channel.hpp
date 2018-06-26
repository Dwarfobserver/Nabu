
#pragma once

#include <common/core.hpp>
#include <common/messages.hpp>
#include <cstdint>
#include <string>
#include <future>

template <class Channel>
class channel_base {
public:
    bool is_empty() const {
        return static_cast<Channel const*>(this)->is_empty();
    }

    template <class T>
    channel_base& operator<<(T const& data) {
        static_cast<Channel*>(this)->send(data);
        return *this;
    }
    
    template <class T>
    channel_base& operator>>(T& data) {
        static_cast<Channel*>(this)->receive(data);
        return *this;
    }
};


#if defined(MERGE_CLIENT_SERVER)
    #include <common/detail/channel_merge.inl>
#else
    #include <common/detail/channel_split.inl>
#endif


class reactor_base;


channel make_channel(person_id const& src, person_id const& dst);
std::future<channel> connect_with(reactor_base& reactor, person_id const& src, person_id const& dst);

