#include "tcp_connection.hpp"
#include "tcp_socket.hpp"
#include "tcp_listener.hpp"

using namespace boost;
using namespace asio;
using namespace ip;
using namespace std;

namespace tcp_udp_listener {

    void tcp_connection::start() {
        socket_.async_read_some(buffer(rcv_buffer_),
            bind(&tcp_connection::receive_callback, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void tcp_connection::respond(string data) {
        boost::asio::async_write(socket_, buffer(data, data.length()),
            boost::bind(&tcp_connection::send_callback, this, boost::asio::placeholders::error));
    }

    void tcp_connection::receive_callback(const system::error_code& error, size_t bytes_transferred) {        
        if (error) delete this;
        else 
            tcp_socket_->process_received_data(string(rcv_buffer_.data(), rcv_buffer_.data() + bytes_transferred), this);        
    }

    void tcp_connection::send_callback(const boost::system::error_code& error) {
        delete this;
    }

    tcp::socket& tcp_connection::socket() {
        return socket_;
    }
}