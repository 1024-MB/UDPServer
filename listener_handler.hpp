#include <boost/thread.hpp>

namespace udp_server {

	class udp_listener;

	class listener_handler {
	public: 
		static const std::string help_flag, open_port_flag, close_port_flag, cancel_flag;
		static const std::string display_status_flag;
		static const unsigned short min_port = 1024, max_port = 65535;

	public:
		static bool validate_flag(std::string flag);
		static bool validate_port(long number);
		static std::string invalid_port_message();
		void run();		
	public:
		listener_handler(std::vector<std::string> flags, std::vector<unsigned short> ports);
		~listener_handler() {
			delete listener;
			delete runner_flag;
		}

	private:
		std::vector<unsigned short> ports;
		std::vector<std::string> flags;
		udp_listener* listener;
		boost::thread thread;
		bool* runner_flag;

	private:
		void start_listener();
		void action(std::string input);
		void display_listener_status();
		void run_listener(udp_listener* listener, std::vector<unsigned short> ports, bool* runner_flag);
		void read_port(std::string flag);
		void display_help();
		void display_data(std::string data);
		void display_log(std::stringstream& stream);
		bool handle_open_port(unsigned short port);
		bool handle_close_port(unsigned short port);
	};
}