
#pragma once

#include <common/core.hpp>
#include <stdexcept>


template <class T>
constexpr int serialized_size(T const& data) {
    return sizeof(T);
}

template <class T>
int serialize(T const& data, char* buffer) {
    int size = serialized_size(data);
    std::memcpy(buffer, &data, size);
    return size;
}

template <class T>
int deserialize(T& data, char const* buffer) {
    int size = serialized_size(data);
    std::memcpy(&data, buffer, size);
    return size;
}


class connection_error : public std::runtime_error {
public:
    connection_error(std::string const& str) :  std::runtime_error{str} {}
};

namespace msg {

    struct connect_request {
        person_id src;
    };

    struct connect_response {
        bool accepted;
        person_id id;
    };

}
