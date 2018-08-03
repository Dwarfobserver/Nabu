
#pragma once

#include <environment.hpp>
#include <msg/parser.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <chrono>
#include "logger.hpp"


namespace nabu::net {

class socket {
    sf::TcpSocket socket_;
	std::vector<std::byte> buffer_;
	msg::parser parser_;
    std::chrono::steady_clock::time_point last_beat_;
    bool connected_;
public:
    socket() :
		buffer_(1024),
		connected_{ false }
    {
	    socket_.setBlocking(false);
    }

	bool try_connect(sf::IpAddress const& ip, uint16_t const port) {
		connected_ = socket_.connect(ip, port) == sf::Socket::Done;
		return connected_;
    }

	template <class Message>
	void send(Message const& msg) {
		NABU_ASSERT(is_connected(), "Tried to send a {} while being not connected", name_of<Message>());

		auto span = throw_stream{ buffer_ };
		parser_.serialize(span, msg);
		socket_.send(span.begin, span.size()); // TODO Check result
	}

	void receive_all() {
		NABU_ASSERT(is_connected(), "Tried to receive messages while being not connected");

		size_t received = 0;
		socket_.receive(buffer_.data(), buffer_.size(), received); // TODO Check result
		auto span = throw_stream{ buffer_.data(), received };
		try {
			while (!span.is_empty()) {
				parser_.deserialize(span);
			}
		}
		catch (std::runtime_error const& e) {
			logger.warning("While receiving data from socket : {}", e.what());
		}
    }

	template <class F>
	void add_callback(F&& f)
    {
		parser_.set_callback(std::forward<F>(f));
    }

    bool is_connected() const noexcept { return connected_; }
};

} // nabu::net
