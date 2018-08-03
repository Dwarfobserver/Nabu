
#include <net/server.hpp>
#include <terminal.hpp>
#include <database.hpp>
#include <future>
#include "reflection.hpp"


int main() {
    try {
		using namespace nabu;
		logger.info("Nabu server v{} - {}", NABU_VERSION_STRING, NABU_BUILD_TYPE_STRING);

        nabu::terminal terminal;
        net::server    server { 43210 };
        auto& db = database;

        bool quit = false;
        terminal.commands["quit"] = [&] (auto&) {
            quit = true;
        };

        while (!quit) {
            using namespace std::literals;
            server.update();
            terminal.update();
            std::this_thread::sleep_for(20ms);
        }
    }
    catch (std::exception const& e) {
	    nabu::logger.error("Error '{}' caught in main : {}", nabu::name_of(e), e.what());
        return 1;
    }
}
