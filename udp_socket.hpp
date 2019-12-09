#include <boost/array.hpp>

namespace udp_server {

	class udp_listener;

	class udp_socket {
	public:
		const unsigned short port;

	public:
		void listen();
		void close();
		void confirm(boost::shared_ptr<std::string> response);
		boost::asio::ip::basic_endpoint<boost::asio::ip::udp> endpoint();
		std::string data(size_t bytes);

	public:
		udp_socket(boost::asio::io_context* context, unsigned short port, udp_listener* listener) 
			: port(port), socket_(*context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)), listener_(listener) {}

	private:
		boost::asio::ip::udp::socket socket_;
		boost::asio::ip::udp::endpoint endpoint_;
		boost::array<char, 1024> rcv_buffer_;
		udp_listener* listener_;

		void receive_callback(const boost::system::error_code& error, std::size_t bytes_transferred);
		void send_callback(boost::shared_ptr<std::string> message, const boost::system::error_code& error, std::size_t bytes_transferred);
	};
}
