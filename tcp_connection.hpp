#include <boost/asio.hpp>
#include <boost/array.hpp>

namespace tcp_udp_listener {

    class tcp_socket;

    class tcp_connection {
    public:
        void start();
        boost::asio::ip::tcp::socket& socket();
        void respond(std::string data);

    public:
        tcp_connection(boost::asio::io_service& io_service, tcp_socket* tcp_socket) : tcp_socket_(tcp_socket), socket_(io_service) {}

    private:
        tcp_socket* tcp_socket_;
        boost::asio::ip::tcp::socket socket_;
        boost::array<char, 1024> rcv_buffer_;

    private:
        void receive_callback(const boost::system::error_code& error, size_t bytes_transferred);
        void send_callback(const boost::system::error_code& error);
    };
}