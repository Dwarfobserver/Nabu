
#pragma once

#include <msg/parser.hpp>
#include <logger.hpp>
#include <function2.hpp>
#include <SFML/Network.hpp>
#include <memory>


namespace nabu::net {

class server {
public:
	explicit server(uint16_t const port);
    void update();

	fu2::unique_function<void()> on_welcome;
	fu2::unique_function<void()> on_message;
private:
	sf::TcpListener listener_;
	std::unique_ptr<sf::TcpSocket> next_socket_;
	std::vector<std::unique_ptr<sf::TcpSocket>> sockets_;
	msg::parser parser_;
	std::vector<std::byte> buffer_;

	void update_new_connection();
	void update_connections();
};

} // nabu::net
