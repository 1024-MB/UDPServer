#include <iostream>
#include <boost/lexical_cast.hpp>
#include "listener_handler.hpp";
#include "udp_listener.hpp"

using namespace std;
using namespace std::chrono;

namespace udp_server {

    const string
        listener_handler::help_flag = "-h",
        listener_handler::start_flag = "-s",
        listener_handler::open_port_flag = "-po",
        listener_handler::close_port_flag = "-pc",
        listener_handler::cancel_flag = "-c",
        listener_handler::display_status_flag ="-ds";

	listener_handler::listener_handler(vector<string> flags, vector<unsigned short> ports) {
        this->flags = flags;
        this->ports = ports;

        runner_flag = new bool(false);
		listener = new udp_listener();
        listener->subscribe_logger(&listener_handler::display_log, *this);
        listener->subscribe_data_reader(&listener_handler::display_data, *this);

        cout << "Welcome to UDP Server v1.0" << endl;
        cout << "Use -h to list available options" << endl << endl;
	}

    void listener_handler::run() {
        if (any_of(flags.begin(), flags.end(), [](string flag) { return flag == "-s"; }))
            start_listener();

        string input;
        while (true) {
            cin >> input;
            action(input);
        }
    }

	void listener_handler::start_listener() {			
		thread = boost::thread { &listener_handler::run_listener, this, listener, ports, runner_flag };

        auto begin = system_clock::now();
        int64_t duration;
        do {
            auto current = system_clock::now();
            duration = duration_cast<std::chrono::milliseconds>(current - begin).count();
        } while (*runner_flag == false && duration < 5000);

        if (*runner_flag == true || ports.empty())
            display_listener_status();
        else throw runtime_error("Unexpected listener behaviour, shutting down.");        
	}

    void listener_handler::run_listener(udp_listener* listener, vector<unsigned short> ports, bool* runner_flag) {
        try {
            for (int i = 0; i < ports.size(); i++)
                listener->listen_on(ports.at(i));

            *runner_flag = true;
            listener->run();
        }
        catch (...) {
        }
        *runner_flag = false;
    }

    void listener_handler::display_listener_status() {
        auto num = ports.size();
        if (num > 0) {
            cout << "Listening on UDP: " << flush;
            for (int i = 0; i < num; i++) {
                cout << ports.at(i) << flush;
                if (num - i > 1)
                    cout << ", " << flush;
            }            
        }
        else cout << "No UDP ports are beeing listened" << flush;
        cout << endl <<endl;
    }

    void listener_handler::open_port() {        
        while (true) {
            cout << "Port number: " << flush;
            string input; cin >> input;
            if (input == cancel_flag)
                break;
            try {
                auto num = boost::lexical_cast<long>(input);
                auto it = find_if(ports.begin(), ports.end(), [&](unsigned short port) {return port == num; });
                if (it != ports.end())
                    cout << "Port already listed" << endl;
                else if (!validate_port(num)) 
                    cerr << invalid_port_message() << endl;                
                else {
                    handle_open_port(num);
                    cout << "Port " + to_string(num) + " listed" << endl;
                    if (*runner_flag == true)
                        display_listener_status();
                    break;
                }
            }
            catch (boost::bad_lexical_cast&) {
                cerr << "Invalid input" << endl;
            }
        }
    }

    void listener_handler::handle_open_port(unsigned short port) {
        ports.push_back(port);

        if (*runner_flag == true) {
            listener->listen_on(port);
            listener->restart();
        }
    } 

    void listener_handler::close_port() {
        while (true) {
            cout << "Port number: " << flush;
            string input; cin >> input;
            if (input == cancel_flag)
                break;
            try {
                auto num = boost::lexical_cast<long>(input);
                auto it = find_if(ports.begin(), ports.end(), [&](unsigned short port) {return port == num; });
                if (!validate_port(num))
                    cerr << "Invalid port" << endl;
                else if (it == ports.end())
                    cout << "Port not listed" << endl;
                else {
                    handle_close_port(it);
                    if (*runner_flag == true)
                        display_listener_status();
                    else cout << "Port closed." << endl;
                    break;
                }
            }
            catch (boost::bad_lexical_cast&) {
                cerr << "Invalid input" << endl;
            }
        }
    }

    void listener_handler::handle_close_port(vector<unsigned short>::iterator iterator) {
        listener->stop_listening_on(*iterator);
        ports.erase(iterator);
    }

    void listener_handler::action(string input) {
        if (input == help_flag)
            display_help();
        else if (input == start_flag) {
            if (ports.empty())
                cout << "There are no opened ports to listen on" << endl;
            else start_listener();
        }
        else if (input == open_port_flag)
            open_port();
        else if (input == close_port_flag)
            close_port();
        else if (input == display_status_flag)
            display_listener_status();
    }

    void listener_handler::display_help() {
        cout << endl << start_flag + " Start listener"
            << endl << open_port_flag + " Open port"
            << endl << close_port_flag + " Close port"            
            << endl << cancel_flag + " Cancel input"
            << endl << help_flag + " Help"
            << endl << endl;
    }

    bool listener_handler::validate_flag(string flag) {
        return flag == listener_handler::start_flag;
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
}
	