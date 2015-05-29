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
            //cout << "USAGE: client IP-address port" << endl;
            as3::println("USAGE: client IP-address port");
            exit(1);
        }
    }

    auto connect(as3::Socket const& s, sockaddr_in const& remote_addr) -> void
    {
        if (0 != ::connect(s.get(), (sockaddr*)&remote_addr, sizeof(remote_addr)))
        {
            //cout << "connect failed\n";
            as3::println("connect failed");
            exit(1);
        }
    }
}//namespace as3


auto main(int argc, char *argv[]) -> int
{
    as3::handle_user_input(argc);
    
    as3::setup_win_sock_api(as3::WSVERS);
    auto remote_addr = as3::make_remote_address(argv);
    as3::Socket sock{ AF_INET, SOCK_STREAM, 0 };
    
    as3::connect(sock, remote_addr);
    auto receive = as3::Receive{};
    auto pk_msg = receive(sock);
    auto n = as3::rsa::read_char_as_int(pk_msg[3]);
    auto e = as3::rsa::read_char_as_int(pk_msg[4]);
    as3::println("public key : ") << n << " " << e << endl;
    auto pub_key = as3::rsa::BinKey{ n, e };

    for (auto input = string{}; cin >> input && input != "."; /* */)
    {
        for (auto& ch : input) ch = pub_key(ch);
        as3::send(sock.get(), input + "\r\n");
        as3::println(receive(sock));
    }

    return 0;
}