#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace tcp_udp_listener {

    class tcp_connection;
    class tcp_listener;

    class tcp_socket {
    public:
        const unsigned short port;

    public:
        void listen();
        void process_received_data(std::string data, tcp_connection* connection);

    public:
        tcp_socket(boost::asio::io_context* io_context, unsigned short port, tcp_listener* listener)
            : port(port), io_context_(io_context), acceptor_(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), listener_(listener) {}

    private:
        void initiate_connection_callback(tcp_connection* connection, const boost::system::error_code& error);

    private:
        boost::asio::io_context* io_context_;
        boost::asio::ip::tcp::acceptor acceptor_;
        tcp_listener* listener_;
    };    
}
