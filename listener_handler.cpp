#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "listener_handler.hpp"
#include "udp_listener.hpp"

using namespace std;
using namespace std::chrono;

namespace udp_server {

    const string
        listener_handler::help_flag = "-h",
        listener_handler::open_port_flag = "-a",
        listener_handler::close_port_flag = "-r",
        listener_handler::cancel_flag = "--",
        listener_handler::display_status_flag = "-ds",
        listener_handler::toggle_log_output_flag = "-tl",
        listener_handler::toggle_data_output_flag = "-td";


    listener_handler::listener_handler(vector<string> flags, vector<unsigned short> ports) {
        this->flags = flags;
        this->ports = ports;

        runner_flag = new bool(false);
        listener = new udp_listener();

        if (any_of(flags.begin(), flags.end(), [](string flag) { return flag == toggle_log_output_flag; }))
            toggle_log_output();

        if (any_of(flags.begin(), flags.end(), [](string flag) { return flag == toggle_data_output_flag; }))
            toggle_data_output();

        cout << "UDPListener v1.0" << endl;
        cout << "Use -h to list available options" << endl << endl;
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
        if (!ports.empty()) {
            thread = boost::thread{ &listener_handler::run_listener, this, listener, ports, runner_flag };

            auto begin = system_clock::now();
            int64_t duration;
            do {
                auto current = system_clock::now();
                duration = duration_cast<std::chrono::milliseconds>(current - begin).count();
            } while (*runner_flag == false && duration < 5000);

            if (*runner_flag == true || ports.empty())
                display_listener_status();
            else throw runtime_error("Unexpected listener behaviour, shutting down");
        }
        else cout << "Enlist ports to start listener" << endl << endl;;
    }

    void listener_handler::run_listener(udp_listener* listener, vector<unsigned short> ports, bool* runner_flag) {
        try {
            for (size_t i = 0; i < ports.size(); i++)
                listener->listen_on(ports.at(i));

            *runner_flag = true;
            listener->run();
        }
        catch (runtime_error& ex) {
            cerr << ex.what() << endl;
        }
        *runner_flag = false;
    }

    void listener_handler::display_listener_status() {
        auto num = ports.size();
        if (num > 0 && *runner_flag == true) {
            cout << "Listening on UDP: " << flush;
            for (size_t i = 0; i < num; i++) {
                cout << ports.at(i) << flush;
                if (num - i > 1)
                    cout << ", " << flush;
            }
        }
        else cout << "No UDP ports are beeing listened" << flush;
        cout << endl << endl;
    }    

    void listener_handler::read_port(string flag) {
        while (true) {
            cout << "Port number: " << flush;
            string input; cin >> input;
            if (input == cancel_flag)
                break;
            try {
                auto num = boost::lexical_cast<long>(input);
                if ((flag == open_port_flag) 
                    ? handle_open_port(static_cast<unsigned short>(num)) 
                    : handle_close_port(static_cast<unsigned short>(num)))
                    break;                
            }
            catch (boost::bad_lexical_cast&) {
                cerr << "Invalid input" << endl;
            }
        }
    }

    bool listener_handler::handle_open_port(long num) {
        if (!validate_port(num))
            cerr << invalid_port_message() << endl;

        auto it = find_if(ports.begin(), ports.end(), [&](unsigned short port) { return port == num; });
        if (it != ports.end())
            cout << "Port already listed" << endl;
        else {
            ports.push_back(static_cast<unsigned short>(num));
            cout << "Port " + to_string(num) + " listed" << endl;

            if (*runner_flag == true) {
                listener->listen_on(static_cast<unsigned short>(num));
                listener->restart();
                display_listener_status();
            }
            else start_listener();
            return true;
        }
        return false;
    }

    bool listener_handler::handle_close_port(long num) {
        if (!validate_port(num))
            cerr << "Invalid port number" << endl;

        auto it = find_if(ports.begin(), ports.end(), [&](unsigned short port) {return port == num; });        
        if (it == ports.end())
            cout << "Port not listed" << endl;
        else {
            bool is_listened = *runner_flag;
            listener->stop_listening_on(static_cast<unsigned short>(num));
            ports.erase(it);
            if (is_listened)
                cout << "Port closed" << endl;
            else cout << "Port unlisted" << endl;
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
        try {
            if (boost::starts_with(input, open_port_flag + " ")) {
                auto port = boost::lexical_cast<long>(input.substr(open_port_flag.length() + 1));
                handle_open_port(port);
                return true;
            }
            else if (boost::starts_with(input, close_port_flag + " ")) {
                auto port = boost::lexical_cast<long>(input.substr(input.find(close_port_flag)));
                handle_close_port(port);
                return true;
            }
        }
        catch (boost::bad_lexical_cast&) {
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
        outputting_log ^= 1;
        if (outputting_log) {
            listener->subscribe_logger(&listener_handler::display_log, *this);
            cout << "Outputting log enabled" << endl << endl;;
        }
        else {
            listener->unsubscribe_logger();
            cout << "Outputting log disabled" << endl << endl;;
        }
    }

    void listener_handler::toggle_data_output() {
        outputting_data ^= 1;
        if (outputting_data) {
            listener->subscribe_data_reader(&listener_handler::display_data, *this);
            cout << "Outputting data enabled" << endl << endl;;
        }
        else {
            listener->unsubscribe_data_reader();
            cout << "Outputting data disabled" << endl << endl;;
        }
    }

    listener_handler::~listener_handler() {
        delete listener;
        delete runner_flag;
    }
}
	