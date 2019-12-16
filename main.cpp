#include <iostream>
#include <boost/lexical_cast.hpp>
#include "listener_handler.hpp"

using namespace std;
using namespace tcp_udp_listener;

pair<vector<string>, vector<pair<string, unsigned short>>> parse_arguments(int argc, char* argv[]) {
    vector<string> flag_arguments;
    vector<pair<string, unsigned short>> port_arguments;

    string protocol;
    for (int i = 0; i < argc - 1; i++) {
        string arg = argv[i + 1];

        try {
            auto port = boost::lexical_cast<long>(arg);
            if (!listener_handler::validate_port(port))
                throw runtime_error(listener_handler::invalid_port_message());
            else {
                if(protocol.empty())
                    throw runtime_error(listener_handler::invalid_protocol_message());
                port_arguments.push_back(pair<string, unsigned short>(protocol, static_cast<unsigned short>(port)));
                protocol.clear();
            }
        }
        catch (boost::bad_lexical_cast&) {
            if (listener_handler::validate_flag(arg))
                flag_arguments.push_back(arg);
            else if (listener_handler::validate_protocol(arg))
                protocol = arg;
            else throw runtime_error("Invalid argument: " + arg);
        }   
    }

    return pair<vector<string>, vector<pair<string, unsigned short>>>(flag_arguments, port_arguments);
}

int main(int argc, char* argv[])
{
    try {
        auto arguments = parse_arguments(argc, argv);
        listener_handler handler(arguments.first, arguments.second);
        handler.run();

        return 0;
    }
    catch (runtime_error & ex) {
        cerr << ex.what() << endl;
        return 1;
    }
}