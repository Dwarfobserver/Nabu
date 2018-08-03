
#pragma once

#include <logger.hpp>
#include <function2.hpp>
#include <future>
#include <iostream>
#include <string>
#include <sstream>
#include <map>


namespace nabu {

class terminal {
    std::future<std::string> cin_task;

    inline void make_task() {
        cin_task = std::async(std::launch::async, [] {
            std::string line;
            std::getline(std::cin, line);
            return line;
        });
    }
public:
    using command_t = fu2::unique_function<void(std::string const&)>;

    std::map<std::string, command_t> commands;

    inline terminal() {
        make_task();
    }

    inline void update() {
        using namespace std::literals;
        if (cin_task.wait_for(0s) != std::future_status::ready) return;

        auto const str = cin_task.get();
        make_task();
        if (str.empty()) return;

        auto space = str.find_first_of(' ');

        auto const cmd = str.substr(0, space);
        auto const arg = str.substr(space + 1);

        auto& f = commands[cmd];
        if (f.empty()) {
            logger.warning("invalid '{}' command", cmd);
        }
        else f(arg);
    }

    inline ~terminal() {
        logger.info("Press a touch to exit");
        cin_task.wait();
    }
};

} // nabu
