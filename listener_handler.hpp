#include <boost/thread.hpp>
#include <boost/asio.hpp>

namespace tcp_udp_listener {

	class tcp_listener;
	class udp_listener;

	class listener_handler {
	public: 
		static const std::string help_flag, open_port_flag, close_port_flag, cancel_flag;
		static const std::string display_status_flag, toggle_log_output_flag, toggle_data_output_flag;
		static const std::string tcp_protocol, udp_protocol;
		static const unsigned short min_port, max_port;

	public:
		static bool validate_flag(std::string flag);
		static bool validate_port(long number);
		static bool validate_protocol(std::string protocol);
		static std::string invalid_port_message();
		static std::string invalid_protocol_message();
		void run();

	public:
		listener_handler(std::vector<std::string> flags, std::vector<std::pair<std::string, unsigned short>> ports);
		~listener_handler();

	private:
		boost::asio::io_context* context_;
		std::vector<unsigned short> tcp_ports_;
		std::vector<unsigned short> udp_ports_;
		tcp_listener* tcp_listener_;
		udp_listener* udp_listener_;
		boost::thread thread_;
		bool* runner_flag_;
		bool outputting_log_{ false };
		bool outputting_data_{ false };

	private:
		void start_listener();
		void action(std::string input);
		void display_listener_status();
		void run_listener();
		void read_port(std::string flag);
		void display_help();
		void display_data(std::string data);
		void display_log(std::stringstream& stream);
		void toggle_log_output();
		void toggle_data_output();
		bool single_line_action(std::string input);
		bool handle_open_port(long num, std::string protocol);
		bool handle_close_port(long num, std::string protocol);
	};
}