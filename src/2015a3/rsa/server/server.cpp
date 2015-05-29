#include <stdlib.h>
#include <winsock.h>
#include "../lib/as3.hpp"
#include <string>
using std::string; using std::to_string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

namespace as3
{
    auto inline make_address() -> sockaddr_in
    {
        auto addr = sockaddr_in{};
        memset(&addr, 0, sizeof(addr));
        return addr;
    }

    auto inline bind(as3::Socket const& sock, sockaddr_in const& addr) -> bool
    {
        if ( 0 != ::bind(sock.get(), (struct sockaddr *)(&addr), sizeof(addr)) )
        {
            as3::println("Bind failed!");
            return false;
        }
        return true;
    }

    namespace rsa
    {
        auto make_rsa_key_list() -> as3::rsa::RsaKeyList
        {
            auto keys = { TriKey{ 143, 7, 103 }, TriKey{ 187, 27, 83 }, TriKey{ 209, 17, 53 } };
            return RsaKeyList(keys.begin(), keys.end());
        }
    }
}

int main(int argc, char *argv[])
{
    as3::setup_win_sock_api(as3::WSVERS);
    as3::Socket sock{ AF_INET, SOCK_STREAM, 0 };
    if (sock.is_failed()) as3::println( "socket failed" );

    auto local_addr = as3::make_address();
    {
        local_addr.sin_family = AF_INET;
        if (argc == 2) local_addr.sin_port = htons((u_short)atoi(argv[1]));
        else local_addr.sin_port = htons(1234);//default listening port
        local_addr.sin_addr.s_addr = INADDR_ANY;//server IP address should be local
    }

    if (as3::bind(sock, local_addr) == false) return 1;
    auto remote_addr = as3::make_address();

    auto key_list = as3::rsa::make_rsa_key_list();
    for (listen(sock.get(), 5); true; as3::println("disconnected from " + string(inet_ntoa(remote_addr.sin_addr))))
    {
        int addrlen = sizeof(remote_addr);
        as3::Socket new_sock{ ::accept(sock.get(), (sockaddr*)(&remote_addr), &addrlen) };
        if (new_sock.is_failed()) break;
        as3::println("accepted connection from IP " + string(inet_ntoa(remote_addr.sin_addr)) + " port " + to_string(ntohs(remote_addr.sin_port)));
        
        as3::send(new_sock.get(), string("pk=") + key_list.current_public_key().to_str() + "\r\n");
        auto prv_key = key_list.current_private_key();
        for (auto receive = as3::Receive{};;)
        {
            auto msg_en = receive(new_sock);
            if (!receive.is_normal()) break;
            as3::println("The original was:\n'" + msg_en + "'");
            auto msg_de = msg_en;
            for (auto& ch : msg_de) 
                ch = prv_key.calculate(as3::rsa::read_char_as_int(ch));
            as3::println("The decrypted is:\n'" + msg_de + "'");

            auto feed_back = "<<< SERVER SAYS:The client typed '" + msg_en + "' -- " + to_string(msg_en.size()) + " bytes in total\r\n";
            as3::send(new_sock.get(), feed_back);
        }
    }
    return 0;
}