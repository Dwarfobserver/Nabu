
#include <configuration.hpp>
#include <fmt/format.h>
#include <iostream>
#include <filesystem>


namespace {

enum class symbol {
    section,
    string_value,
    number_value,
    empty
};

struct parser_context {
    symbol type = symbol::empty;
    std::string section;
    std::string key;
    std::string value;
};

struct parser_result;
using parser_t = parser_result (*) (parser_context&, char);
struct parser_result {
    parser_t self;
};

// Parse the key in 'key = value'.
parser_result parse_key(parser_context& ctx, char c);

// Parse the string or number value in 'key = value'.
parser_result parse_string(parser_context& ctx, char c);
parser_result parse_number(parser_context& ctx, char c);

// Parse the end of a line.
parser_result parse_end(parser_context& ctx, char c);

// Parse the section in '[section]'.
parser_result parse_section(parser_context& ctx, char c);

// Parse before and after the equal sign in 'key = value'.
parser_result parse_separator_1(parser_context& ctx, char c);
parser_result parse_separator_2(parser_context& ctx, char c);

// Parse the beginning of a line.
parser_result parse_begin(parser_context& ctx, char c);

parser_result parse_separator_1(parser_context& ctx, char const c) {
    switch (c) {
        case ' ' : return { parse_separator_1 };
        case '=' : return { parse_separator_2 };
        default  : throw std::string { c };
    };
}

parser_result parse_separator_2(parser_context& ctx, char const c) {
    switch (c) {
        case ' ' : return { parse_separator_2 };
        case '"' : ctx.type = symbol::string_value; return { parse_string };
        default  : if (c >= '0' && c <= '9')
        {
            ctx.type = symbol::number_value;
            ctx.value.push_back(c);
            return { parse_number };
        }
        else throw c;
    };
}

parser_result parse_begin(parser_context& ctx, char const c) {
    switch (c) {
        case '\0': return { nullptr };
        case '#' : return { nullptr };
        case ' ' : return { parse_begin };
        case '=' : throw std::string { c };
        case '"' : throw std::string { c };
        case '[' : ctx.type = symbol::section; return { parse_section };
        case ']' : throw std::string { c };
        default  : ctx.key.push_back(c); return { parse_key };
    };
}

parser_result parse_end(parser_context& ctx, char const c) {
    switch (c) {
        case '\0': return { nullptr };
        case '#' : return { nullptr };
        case ' ' : return { parse_end };
        default  : throw std::string { c };
    };
}

parser_result parse_section(parser_context& ctx, char const c) {
    switch (c) {
        case '\0': throw std::string { "end of line" };
        case '#' : throw std::string { c };
        case '=' : throw std::string { c };
        case '"' : throw std::string { c };
        case '[' : throw std::string { c };
        case ']' : return { parse_end };
        default  : ctx.section.push_back(c); return { parse_section };
    };
}

parser_result parse_key(parser_context& ctx, char const c) {
    switch (c) {
        case '\0': throw std::string { "end of line" };
        case '#' : throw std::string { c };
        case ' ' : return { parse_separator_1 };
        case '=' : return { parse_separator_2 };
        case '"' : throw std::string { c };
        case '[' : throw std::string { c };
        case ']' : throw std::string { c };
        default  : ctx.key.push_back(c); return { parse_key };
    };
}

parser_result parse_string(parser_context& ctx, char const c) {
    switch (c) {
        case '\0': throw std::string { "end of line" };
        case '"' : return { parse_end };
        default  : ctx.value.push_back(c); return { parse_string };
    };
}

parser_result parse_number(parser_context& ctx, char const c) {
    switch (c) {
        case '\0': return { nullptr };
        case '#' : return { nullptr };
        case ' ' : return { parse_end };
        default  : if (c >= '0' && c <= '9')
        {
            ctx.value.push_back(c);
            return { parse_number };
        }
        else throw std::string { c };
    };
}

} // <anonymous>

namespace nabu {

toml_file::toml_file(std::string_view file_name) :
    name{ file_name }
{
    auto num_character = 0;

    auto parse_line =
        [&num_character, this, section_ptr = decltype(&sections[""]){}]
        (std::string const& line, int num_line) mutable
    {
        auto const str = line.c_str();
        auto ctx       = parser_context{};
        num_character  = 0;
        auto parser    = parse_begin  (ctx, str[  num_character]).self;
        while (parser) parser = parser(ctx, str[++num_character]).self;

        switch (ctx.type) {
            case symbol::empty: break;
            case symbol::section: section_ptr = &sections[ctx.section]; break;
            default:
                if (section_ptr == nullptr) throw "'key = value' line (put a '[section]' first)";
                auto& value = (*section_ptr)[ctx.key];
                if (ctx.type == symbol::number_value)
                     value = std::stoi(ctx.value);
                else value = ctx.value;
        }
    };

    auto file = std::ifstream{ name };
    auto line = std::string{};
    auto num_line = 0;
    while (std::getline(file, line)) try {
        parse_line(line, ++num_line);
    } catch (std::string const& err) {
        std::cout << "Error while parsing '" << name << "' :\n";
        std::cout << "Line " << num_line << " :\n";
        std::cout << line << '\n';
        std::cout << std::string(num_character, ' ') << "^\n";
        std::cout << "Unexpected " << err << '\n';
        std::terminate();
    }
}

namespace {
auto const default_config_file = 
R"(
# Nabu configuration file.

[server]
ip   = "127.0.0.1"
port = 43210

[logger]
verbosity = "debug"
output    = "console" # "console" redirects on std::cout.
)";
}

namespace detail {
    NABU_IMPLEMENT_GLOBAL(config_file, [] {
        namespace fs = std::filesystem;
        auto const file_name = "nabu_config.toml";
        try {
            if (!fs::exists(fs::current_path() / fs::path{ file_name })) {
                std::cout << "Creating new " << file_name << '\n';
                std::ofstream file{ file_name };
                file << default_config_file;
            }else std::cout << "Opening " << file_name << '\n';
        }
        catch (std::exception const& e) {
            std::cout << "Error while creating config_file : " << e.what() << '\n';
            std::terminate();
        }
        return file_name;
    } ());
}

#define ASSERT_IT(it, msg) \
    if (server_it == toml.sections.end()) \
        throw "nabu_config.toml is missing " msg

configuration_type::configuration_type(toml_file const& toml) {
    try {
        auto const server_it = toml.sections.find("server");
        ASSERT_IT(server_it, "[server] section");
        auto& server_map = server_it->second;
        {
            auto const server_ip = server_map.find("ip");
            ASSERT_IT(server_ip, "server.ip");
            server.ip = std::get<std::string>(server_ip->second);
            
            auto const server_port = server_map.find("port");
            ASSERT_IT(server_port, "server.port");
            server.port = std::get<int>(server_port->second);
        }

        auto const logger_it = toml.sections.find("logger");
        ASSERT_IT(logger_it, "[logger] section");
        auto& logger_map = logger_it->second;
        {
            auto const logger_verbosity = logger_map.find("verbosity");
            ASSERT_IT(logger_verbosity, "logger.verbosity");
            logger.verbosity = std::get<std::string>(logger_verbosity->second);
            
            auto const logger_output = logger_map.find("output");
            ASSERT_IT(logger_output, "logger.output");
            logger.output = std::get<std::string>(logger_output->second);
        }
    }
    catch (char const* const err) {
        std::cout << err << '\n';
        std::terminate();
    }
}

NABU_IMPLEMENT_GLOBAL(configuration, ::nabu::detail::config_file);

} // nabu
