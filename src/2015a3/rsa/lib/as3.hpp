#pragma once

#include <winsock.h>
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin; using std::ostream;


namespace as3
{
    template<typename Printable>
    auto println(Printable const& printable) -> ostream&{ return cout << printable << endl; }

    const int WSVERS = MAKEWORD(2, 0);

    class Socket
    {
    public:
        explicit Socket(SOCKET s)
            : socket_{ s }
        {}
        Socket(int address_family, int type, int protocol)
            : socket_{ ::socket(address_family, type, protocol) }
        {}

        auto get() const ->  SOCKET { return socket_; }
        auto is_failed() const -> bool { return socket_ < 0; }
        ~Socket(){ ::closesocket(socket_); }

        Socket(Socket const&) = delete;
        Socket(Socket &&) = delete;
        Socket& operator=(Socket const&) = delete;
        Socket& operator=(Socket &&) = delete;
    private:
        const SOCKET socket_;
    };

    auto setup_win_sock_api(WORD version_required) -> WSADATA
    {
        WSADATA wsadata;
        if (WSAStartup(WSVERS, &wsadata) != 0)
        {
            WSACleanup();
            println("WSAStartup failed");
            exit(1);
        }
        return wsadata;
    }

    class Receive
    {
    public:
        Receive() : is_normal_{ true }{}
        auto operator()(Socket const& sock) -> string
        {
            auto received = string();
            for (auto ch = char(0); true; /* */) //receive char by char, end on an LF, ignore CR's
            {
                if (0 >= recv(sock.get(), &ch, 1, 0)) 
                { 
                    is_normal_ = false;
                    println("recv failed"); 
                    break;
                }
                if (ch == '\n') break; else if (ch == '\r') continue; else received.push_back(ch);
            }
            return received;
        }
        auto is_normal() const -> bool { return is_normal_; }
    private:
        bool is_normal_;
    };

    auto send(SOCKET sock, string const& send_buffer) -> void
    {
        auto bytes_sent = ::send(sock, send_buffer.c_str(), send_buffer.size(), 0);
        if (bytes_sent < 0) { println( "send failed" ); exit(1); }
    }

}//namespace