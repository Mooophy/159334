#include <winsock.h>
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

#include "../lib/as3.hpp"

namespace as3
{
    auto make_remote_address(char* arr[]) -> sockaddr_in
    {
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_addr.s_addr = inet_addr(arr[1]);    //IP address
        address.sin_port = htons((u_short)atoi(arr[2]));//Port number
        address.sin_family = AF_INET;
        return address;
    }

    auto handle_user_input(int arguments_count) -> void
    {
        if (arguments_count != 3)
        {
            cout << "USAGE: client IP-address port" << endl;
            exit(1);
        }
    }

    auto connect(as3::Socket const& s, sockaddr_in const& remote_addr) -> void
    {
        if (0 != ::connect(s.get(), (sockaddr*)&remote_addr, sizeof(remote_addr)))
        {
            cout << "connect failed\n";
            exit(1);
        }
    }

    auto send(SOCKET sock, string const& send_buffer) -> void
    {
        auto bytes_sent = ::send(sock, send_buffer.c_str(), send_buffer.size(), 0);
        if (bytes_sent < 0) { cout << "send failed\n"; exit(1); }
    }
}//namespace as3


auto main(int argc, char *argv[]) -> int
{
    //handle arugments
    as3::handle_user_input(argc);
    
    //init
    as3::setup_win_sock_api(as3::WSVERS);
    auto remote_addr = as3::make_remote_address(argv);
    as3::Socket sock{ AF_INET, SOCK_STREAM, 0 };
    
    //connect
    as3::connect(sock, remote_addr);
    for (auto input = string{}; cin >> input && input != "."; /* */)
    {
        as3::send(sock.get(), input + "\r\n");
        cout << as3::receive(sock.get()) << endl;
    }

    return 0;
}