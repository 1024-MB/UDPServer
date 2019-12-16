#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "tcp_socket.hpp"
#include "tcp_connection.hpp"
#include "tcp_listener.hpp"

using namespace boost;
using namespace asio;
using namespace std;

namespace tcp_udp_listener {

	void tcp_socket::listen() {
        auto connection = new tcp_connection(*io_context_, this);

        acceptor_.async_accept(connection->socket(),
            bind(&tcp_socket::initiate_connection_callback, this, connection, asio::placeholders::error));
	}

    void tcp_socket::initiate_connection_callback(tcp_connection* connection, const system::error_code& error) {
        if (!error) {
            connection->start();
            listen();
        }
        else delete connection;
    }

    // development temporary method
    void tcp_socket::process_received_data(std::string data, tcp_connection* connection) {
        if (data == "SIMPLE TIME")
            listener_->receive_time_request(data, connection);
        else if (starts_with(data, "GET "))
            listener_->receive_http_request(data, connection);
        else connection->respond("Bad Request");
    }    
}
