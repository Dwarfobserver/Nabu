
#include <net/socket.hpp>
#include <msg/types.hpp>
#include <reactor.hpp>
#include <terminal.hpp>
#include <logger.hpp>
#include <SFML/Network.hpp>
#include <thread>


int main() {
	try {
		using namespace nabu;
		using namespace std::literals;
		logger.info("Nabu client v{} - {}", NABU_VERSION_STRING, NABU_BUILD_TYPE_STRING);

		nabu::terminal terminal;
		net::socket socket;

		reactor.add_routine([&] {
			terminal.update();
			return false;
		});

		terminal.commands["connect"] = [&](std::string const& args) {
			auto const space = args.find(' ');
			if (space == std::string::npos) {
				logger.warning("Wrong command syntax : 'command <account> <password>'");
				return;
			}
			auto const account = args.substr(0, space);
			auto const password = args.substr(space + 1);
			reactor.add_routine([&, account, password] {
				auto const& address = configuration.server;
				auto const success = socket.try_connect(sf::IpAddress{ address.ip }, address.port);
				if (!success) return false;

				reactor.add_routine([&] {
					socket.receive_all();
					return false;
				});
				socket.add_callback([account] (msg::login_response&& response) {
					switch (response.code) {
					case msg::login_response::success:
						logger.info("connected as '{}'", account); break;
					case msg::login_response::unknown_account:
						logger.info("falied to connect : unknown account '{}'", account); break;
					case msg::login_response::wrong_password:
						logger.info("falied to connect : wrong password"); break;
					}
					return true;
				});
				socket.send(msg::login_request{ account, password });
				return true;
			});
		};
		while (true) {
			reactor.update();
			std::this_thread::sleep_for(20ms);
		}
	}
	catch (std::exception const& e) {
		nabu::logger.error("Error '{}' caught in main : {}", nabu::name_of(e), e.what());
		return 1;
	}
}
