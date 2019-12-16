#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "listener_handler.hpp"
#include "tcp_listener.hpp"
#include "udp_listener.hpp"

using namespace std;
using namespace std::chrono;
using namespace boost;
using namespace asio;

namespace tcp_udp_listener {

    const unsigned short
        listener_handler::min_port = 1024,
        listener_handler::max_port = 65535;

    const string
        listener_handler::tcp_protocol = "-tcp",
        listener_handler::udp_protocol = "-udp";

    const string
        listener_handler::help_flag = "-h",
        listener_handler::open_port_flag = "-a",
        listener_handler::close_port_flag = "-r",
        listener_handler::cancel_flag = "--",
        listener_handler::display_status_flag = "-ds",
        listener_handler::toggle_log_output_flag = "-tl",
        listener_handler::toggle_data_output_flag = "-td";


    listener_handler::listener_handler(vector<string> flags, vector<pair<string, unsigned short>> ports) {
        context_ = new io_context();
        tcp_listener_ = new tcp_listener(context_);
        udp_listener_ = new udp_listener(context_);
        runner_flag_ = new bool(false);

        cout << "TCP/UDP Listener v2.0" << endl;
        cout << "Use -h to list available options" << endl << endl;

        for (size_t i = 0; i < ports.size(); i++) {
            auto port = ports.at(i);
            auto protocol = port.first;
            auto number = port.second;

            if (protocol == tcp_protocol) {
                if (!tcp_listener_->listen_on(number))
                    cerr << "Error: TCP " << to_string(number) << " already in use" << endl;
                else this->tcp_ports_.push_back(number);
            }
            else if (protocol == udp_protocol) {
                if (!udp_listener_->listen_on(number))
                    cerr << "Error: UPD " << to_string(number) << " already in use" << endl;
                else this->udp_ports_.push_back(number);
            } 
            else throw runtime_error(invalid_protocol_message());        
        }

        if (any_of(flags.begin(), flags.end(), [](string flag) { return flag == toggle_log_output_flag; }))
            toggle_log_output();

        if (any_of(flags.begin(), flags.end(), [](string flag) { return flag == toggle_data_output_flag; }))
            toggle_data_output();
    }

    void listener_handler::run() {
        start_listener();

        string input;
        while (true) {
            getline(cin, input);
            action(input);
        }
    }

    void listener_handler::start_listener() {
        if (!tcp_ports_.empty() || !udp_ports_.empty()) {
            thread_ = boost::thread{ &listener_handler::run_listener, this };

            auto begin = system_clock::now();
            int64_t duration;
            do {
                auto current = system_clock::now();
                duration = duration_cast<std::chrono::milliseconds>(current - begin).count();
            } while (*runner_flag_ == false && duration < 5000);

            if (*runner_flag_ == true || (tcp_ports_.empty() && udp_ports_.empty()))
                display_listener_status();
            else throw runtime_error("Unexpected listener behaviour, shutting down");
        }
        else cout << "Enlist ports to start listener" << endl << endl;;
    }

    void listener_handler::run_listener() {
        try {
            *runner_flag_ = true;
            context_->run();
        }
        catch (runtime_error& ex) {
            cerr << ex.what() << endl;
        }
        *runner_flag_ = false;
    }

    void listener_handler::display_listener_status() {
        auto protocol = "TCP";
        auto source = &tcp_ports_;

        for (int i = 0; i < 2; i++) {
            auto num = source->size();
            if (num > 0 && *runner_flag_ == true) {
                cout << "Listening on " << protocol << ": " << flush;
                for (size_t i = 0; i < num; i++) {
                    cout << source->at(i) << flush;
                    if (num - i > 1)
                        cout << ", " << flush;
                }
            }
            else cout << "No " << protocol << " ports are beeing listened" << flush;

            protocol = "UDP";
            source = &udp_ports_;
            cout << endl;
        }
        cout << endl;
    }    

    void listener_handler::read_port(string flag) {
        string protocol, port;
        while (true) {
            cout << "Protocol: " << flush;
            cin >> protocol;
            if (protocol == cancel_flag)
                break;
            else if (iequals(protocol, "tcp"))
                protocol = tcp_protocol;
            else if (iequals(protocol, "udp"))
                protocol = udp_protocol;
            else if (protocol != tcp_protocol || protocol != udp_protocol) {
                cerr << "Invalid input" << endl;
                continue;
            }

            cout << "Port number: " << flush;
            cin >> port;
            if (port == cancel_flag)
                break;

            try {
                auto num = lexical_cast<long>(port);
                if ((flag == open_port_flag) 
                    ? handle_open_port(static_cast<unsigned short>(num), protocol) 
                    : handle_close_port(static_cast<unsigned short>(num), protocol))
                    break;                
            }
            catch (bad_lexical_cast&) {
                cerr << "Invalid input" << endl;
            }
        }
    }

    bool listener_handler::handle_open_port(long num, string protocol) {
        if (!validate_port(num))
            cerr << invalid_port_message() << endl;

        auto source = (protocol == tcp_protocol) 
            ? &tcp_ports_ : (protocol == udp_protocol) 
            ? &udp_ports_ : throw logic_error("Invalid protocol");

        auto it = find_if(source->begin(), source->end(), [&](unsigned short port) { return port == num; });
        if (it != source->end())
            cout << "Port already listed" << endl;
        else {
            auto port = static_cast<unsigned short>(num);
            if ((protocol == tcp_protocol && tcp_listener_->listen_on(num)) || (protocol == udp_protocol && udp_listener_->listen_on(port))) {
                source->push_back(port);
                cout << "Port " + to_string(port) + " listed" << endl << endl;
                if (*runner_flag_ == true) {
                    context_->restart();
                    display_listener_status();
                }
                else start_listener();
                return true;
            }
            else cerr << "Port already in use by another process." << endl;
        }
        return false;
    }

    bool listener_handler::handle_close_port(long num, string protocol) {
        if (!validate_port(num))
            cerr << "Invalid port number" << endl;

        auto source = (protocol == tcp_protocol)
            ? &tcp_ports_ : (protocol == udp_protocol)
            ? &udp_ports_ : throw logic_error("Invalid protocol");

        auto it = find_if(source->begin(), source->end(), [&](unsigned short port) {return port == num; });
        if (it == source->end())
            cout << "Port not listed" << endl;
        else {
            bool is_listened = *runner_flag_;
            if(protocol == tcp_protocol)
                tcp_listener_->stop_listening_on(static_cast<unsigned short>(num));
            else if(protocol == udp_protocol)
                udp_listener_->stop_listening_on(static_cast<unsigned short>(num));
            else throw logic_error("Invalid protocol");

            source->erase(it);   
            cout << "Port closed" << endl << endl;
            display_listener_status();

            return true;
        }
        return false;
    }

    void listener_handler::action(string input) {
        if (single_line_action(input))
            return;
        else if (input == help_flag)
            display_help();
        else if (input == open_port_flag || input == close_port_flag)
            read_port(input);
        else if (input == display_status_flag)
            display_listener_status();
        else if (input == toggle_log_output_flag)
            toggle_log_output();
        else if (input == toggle_data_output_flag)
            toggle_data_output();
        else if (!all_of(input.begin(), input.end(), [](char c) { return isspace(c); }))
            cout << "Unrecognized command \"" + input + "\"" << endl;
    }

    bool listener_handler::single_line_action(string input) {     
        vector<string> tokens;
        split(tokens, input, is_any_of(" "));
        if (tokens.size() != 3)
            return false;

        try {
            auto flag = tokens.at(0);
            auto protocol = tokens.at(1);
            auto port = lexical_cast<long>(tokens.at(2));

            if (iequals(protocol, "tcp"))
                protocol = tcp_protocol;
            else if (iequals(protocol, "udp"))
                protocol = udp_protocol;
            else if (protocol != tcp_protocol || protocol != udp_protocol) 
                return false;

            if (flag == open_port_flag) {
                handle_open_port(port, protocol);
                return true;
            }
            else if (flag == close_port_flag) {
                handle_close_port(port, protocol);
                return true;
            }
        }
        catch (bad_lexical_cast&) {
        }
        return false;
    }

    void listener_handler::display_help() {
        cout 
            << endl << open_port_flag + " Open port"
            << endl << close_port_flag + " Close port"
            << endl << display_status_flag + " Display status"
            << endl << toggle_log_output_flag + " Toggle log output"
            << endl << toggle_data_output_flag + " Toggle data output"
            << endl << cancel_flag + " Cancel input"
            << endl << endl;
    }

    bool listener_handler::validate_flag(string flag) {
        return flag == toggle_log_output_flag || flag == toggle_data_output_flag;
    }

    bool listener_handler::validate_port(long number) {
        return number >= min_port && number <= max_port;
    }

    string listener_handler::invalid_port_message() {
        return "Port argument number must be between " + to_string(min_port) + " and " + to_string(max_port) + " inclusive";
    }

    void listener_handler::display_log(stringstream& stream) {
        cout << stream.str();
    }

    void listener_handler::display_data(string data) {
        cout << data << endl;
    }

    void listener_handler::toggle_log_output() {
        outputting_log_ ^= 1;
        if (outputting_log_) {
            udp_listener_->subscribe_logger(&listener_handler::display_log, *this);
            cout << "Outputting log enabled" << endl << endl;;
        }
        else {
            udp_listener_->unsubscribe_logger();
            cout << "Outputting log disabled" << endl << endl;;
        }
    }

    void listener_handler::toggle_data_output() {
        outputting_data_ ^= 1;
        if (outputting_data_) {
            udp_listener_->subscribe_data_reader(&listener_handler::display_data, *this);
            cout << "Outputting data enabled" << endl << endl;;
        }
        else {
            udp_listener_->unsubscribe_data_reader();
            cout << "Outputting data disabled" << endl << endl;;
        }
    }

    bool listener_handler::validate_protocol(string protocol) {
        return protocol == tcp_protocol || protocol == udp_protocol;
    }

    string listener_handler::invalid_protocol_message() {
        return "Protocol invalid or undefined";
    }

    listener_handler::~listener_handler() {
        delete context_;
        delete tcp_listener_;
        delete udp_listener_;
        delete runner_flag_;
    }
}
	