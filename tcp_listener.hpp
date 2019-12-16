#include <boost/asio.hpp>
#include <boost/signals2.hpp>

namespace tcp_udp_listener {

	class tcp_socket;
    class tcp_connection;

	class tcp_listener {
    public:
        boost::signals2::signal<void(std::string, tcp_connection*)> receive_http_request;
        boost::signals2::signal<void(std::string, tcp_connection*)> receive_time_request;

    public:
        bool listen_on(unsigned short port);
        void stop_listening_on(unsigned short port);

    public:
        tcp_listener(boost::asio::io_context* context);

    private:
        boost::asio::io_context* context_;
        std::vector<tcp_socket*> sockets_;

    private:
        void handle_receive_http_request(std::string data, tcp_connection* connection);
        void handle_receive_time_request(std::string data, tcp_connection* connection);
	};
}
