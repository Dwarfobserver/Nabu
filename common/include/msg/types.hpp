
#pragma once

#include <reflection.hpp>


namespace nabu::msg {

namespace id {
    enum type : id_type {
        login,
        signin,
        disconnect,
        heartbeat
    };
}

struct login_request {
    static constexpr auto id = id::login;
    std::string account;
    std::string password;
};
struct login_response {
    static constexpr auto id = id::login;
    enum {
        success,
        unknown_account,
        wrong_password
    } code;
};

struct signin_request {
    static constexpr auto id = id::signin;
    std::string account;
    std::string password;
};
struct signin_response {
    static constexpr auto id = id::signin;
    enum {
        success,
        account_taken
    } code;
};

struct disconnect {
    static constexpr auto id = id::disconnect;
};

struct heartbeat {
    static constexpr auto id = id::heartbeat;
};

} // nabu::msg
