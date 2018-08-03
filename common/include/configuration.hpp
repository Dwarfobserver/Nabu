
#pragma once

#include <environment.hpp>
#include <fstream>
#include <variant>
#include <vector>
#include <map>


namespace nabu {

class toml_file {
public:
    using value_t = std::variant<std::string, int>;

    std::string name;
    std::map<std::string, std::map<std::string, value_t>> sections;

    toml_file(std::string_view file_name);
};

namespace detail {
    NABU_DECLARE_GLOBAL(toml_file, config_file);
}

struct configuration_type {
    struct server_t {
        std::string ip;
        uint16_t port;
    };
    struct logger_t {
        std::string verbosity;
        std::string output;
    };
    server_t server;
    logger_t logger;

    configuration_type(toml_file const& toml);
};

NABU_DECLARE_GLOBAL(configuration_type const, configuration);

} // nabu
