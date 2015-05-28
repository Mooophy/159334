#include <windows.h>
#include <winsock.h>
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

namespace as3
{
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
        ~Socket(){ ::closesocket(socket_); cout << "clearing socket\n"; }

    private:
        const SOCKET socket_;
    };

    const int WSVERS = MAKEWORD(2, 0);

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

    auto setup_win_sock_api(WORD version_required) -> WSADATA
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

    auto connect(as3::Socket const& s, sockaddr_in const& remote_addr) -> void
    {
        if (0 != ::connect(s.get(), (sockaddr*)&remote_addr, sizeof(remote_addr)))
        {
            cout << "connect failed\n";
            exit(1);
        }
    }

    //return the string received
    auto receive(SOCKET s) -> string
    {
        auto received = string();
        for (auto ch = char(0); true; /* */)//receive char by char, end on an LF, ignore CR's
        {
            if (0 >= recv(s, &ch, 1, 0)) { cout << "recv failed\n"; exit(1); }
            if (ch == '\n') break; else if (ch == '\r') continue; else received.push_back(ch);
        }
        return received;
    }

    auto send(SOCKET sock, string const& send_buffer) -> void
    {
        auto bytes_sent = ::send(sock, send_buffer.c_str(), send_buffer.size(), 0);
        if (bytes_sent < 0) { cout << "send failed\n"; exit(1); }
    }
}//namespace rsa


auto main(int argc, char *argv[]) -> int
{
    //handle arugments
    as3::handle_user_input(argc);
    
    //init
    auto wsa_data = as3::setup_win_sock_api(as3::WSVERS);
    auto remote_addr = as3::make_remote_address(argv);
    auto sock = as3::Socket{ AF_INET, SOCK_STREAM, 0 };
    
    //connect
    as3::connect(sock, remote_addr);
    for (auto input = string{}; cin >> input && input != "."; /* */)
    {
        as3::send(sock.get(), input + "\r\n");
        cout << as3::receive(sock.get()) << endl;
    }
    return 0;
}