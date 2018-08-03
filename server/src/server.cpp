
#include <net/server.hpp>


namespace nabu::net {

namespace {
	std::unique_ptr<sf::TcpSocket> make_socket() {
		auto ptr = std::make_unique<sf::TcpSocket>();
		ptr->setBlocking(false);
		return ptr;
	}
}

server::server(uint16_t const port) :
	on_welcome{ [] {} },
	on_message{ [] {} },
	buffer_(4096)
{
	listener_.listen(port);
	listener_.setBlocking(false);

	next_socket_ = make_socket();
}

void server::update_new_connection() {
	auto& socket = *next_socket_;
	if (listener_.accept(socket) != sf::Socket::Done) return;

	sockets_.push_back(std::move(next_socket_));
	next_socket_ = make_socket();

	logger.debug("new connection in {} from {}:{}",
		socket.getLocalPort(),
		socket.getRemoteAddress().toString(),
		socket.getRemotePort());
}

void server::update_connections() {
	using namespace std::literals;
	char buffer[1024] = {};
	auto i = 0u;
	while (i < sockets_.size()) {
		auto& socket = *sockets_[i];

		size_t received = 0;
		auto const status = socket.receive(buffer, sizeof(buffer), received);

		if (received <= 0) { ++i; continue; }
		buffer[received] = '\0';

		if (status == sf::Socket::Disconnected || buffer == ":DC"s) {
			logger.info("{}:{} has been disconnected",
				socket.getRemoteAddress().toString(),
				socket.getRemotePort());
			sockets_.erase(sockets_.begin() + i);
			continue;
		}

		logger.info("message from {}:{} - {}",
			socket.getRemoteAddress().toString(),
			socket.getRemotePort(),
			buffer);
		++i;
	}
}

void server::update() {
	update_new_connection();
	update_connections();
}

} // nabu::net
