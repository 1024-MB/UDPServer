#include <iostream>
#include "tcp_listener.hpp"
#include "tcp_socket.hpp"
#include "tcp_connection.hpp"

using namespace std;

namespace tcp_udp_listener {

	tcp_listener::tcp_listener(boost::asio::io_context* context) : context_(context) {
        receive_http_request.connect(boost::bind(&tcp_listener::handle_receive_http_request, this, _1, _2));
        receive_time_request.connect(boost::bind(&tcp_listener::handle_receive_time_request, this, _1, _2));
    }

	bool tcp_listener::listen_on(unsigned short port) {
        try
        {
            sockets_.push_back(new tcp_socket(context_, port, this));
            sockets_.back()->listen();
            return true;
        }
        catch (std::runtime_error & ex)
        {
            string error(ex.what());
            if (error == "bind: Address already in use")
                return false;
            else throw ex;
        }
	}

    void tcp_listener::stop_listening_on(unsigned short port) {
        // to be implemented.
    }


    // example test use case
    string make_daytime_string() {
        time_t now = time(0);
        return ctime(&now);
    }

    void tcp_listener::handle_receive_http_request(string data, tcp_connection* connection) {
        connection->respond("HTTP/1.1. 200 OK\r\n\r\n <!DOCTYPE html> <body> " + make_daytime_string() + " </body> </html>");
    }

    void tcp_listener::handle_receive_time_request(string data, tcp_connection* connection) {  
        connection->respond(make_daytime_string());
    }
}