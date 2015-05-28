#pragma once

#include <winsock.h>
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;


namespace as3
{
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

    auto inline setup_win_sock_api(WORD version_required) -> WSADATA
    {
        WSADATA wsadata;
        if (WSAStartup(WSVERS, &wsadata) != 0)
        {
            WSACleanup();
            cout << "WSAStartup failed\n";
            exit(1);
        }
        return wsadata;
    }
}//namespace