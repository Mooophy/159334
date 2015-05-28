#include <windows.h>
#include <winsock.h>
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

namespace rsa
{
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

    //return the string received
    auto receive(SOCKET s) -> string
    {
        auto received = string();
        //receive char by char, end on an LF, ignore CR's
        for (auto ch = char(0); true; /* */)
        {
            if (0 >= recv(s, &ch, 1, 0)) { cout << "recv failed\n"; exit(1); }
            if (ch == '\n') break; else if (ch == '\r') continue; else received.push_back(ch);
        }
        return received;
    }
}//namespace rsa


auto main(int argc, char *argv[]) -> int
{
    rsa::handle_user_input(argc);
    auto wsa_data = rsa::setup_win_sock_api(rsa::WSVERS);
    auto remoteaddr = rsa::make_remote_address(argv);

    //CREATE CLIENT'S SOCKET 
    auto s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        cout << "socket failed\n";
        exit(1);
    }

    //CONNECT
    if (connect(s, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr)) != 0) 
    {
        cout << "connect failed\n";
        exit(1);
    }

    for (auto send_buffer = string{}; cin >> send_buffer; /* */)
    {
        if (send_buffer == ".") break; else send_buffer += "\r\n";

        //SEND
        auto bytes_sent = send(s, send_buffer.c_str(), send_buffer.size(), 0);
        if (bytes_sent < 0) { cout << "send failed\n"; exit(1); }

        //receive
        cout << rsa::receive(s) << endl;
    }

    closesocket(s);
    return 0;
}