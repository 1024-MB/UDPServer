#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "udp_socket.hpp"
#include "udp_listener.hpp"

using namespace boost;
using namespace asio;
using namespace std;
using namespace ip;

namespace tcp_udp_listener {

	void udp_socket::listen() {
		socket_.async_receive_from(buffer(rcv_buffer_), endpoint_,
			bind(&udp_socket::receive_callback, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
	}

	void udp_socket::receive_callback(const system::error_code& error, size_t bytes_transferred) {
		listener_->receive_event_signal(this, bytes_transferred, error);	
		if(!error) listen();		
	}

	void udp_socket::send_callback(boost::shared_ptr<std::string> message, const system::error_code& error, size_t bytes_transferred) {
		listener_->confirm_event_signal(this, error);		
	}

	void udp_socket::confirm(boost::shared_ptr<string> response) {
		socket_.async_send_to(buffer(*response), endpoint_,
			bind(&udp_socket::send_callback, this, response, asio::placeholders::error, asio::placeholders::bytes_transferred));
	}

	basic_endpoint<udp> udp_socket::endpoint(){
		return this->endpoint_;
	}

	void udp_socket::close() {
		socket_.close();
	}

	string udp_socket::data(size_t bytes) {
		return string(rcv_buffer_.data(), rcv_buffer_.data() + bytes);
	}
}