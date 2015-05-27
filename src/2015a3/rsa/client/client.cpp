#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

const int WSVERS = MAKEWORD(2, 0);
const int BUFFESIZE = 200;

auto make_remote_address(char* arr[]) -> struct sockaddr_in
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


auto main(int argc, char *argv[]) -> int
{
    handle_user_input(argc);
    auto wsa_data = setup_win_sock_api(WSVERS);
    auto remoteaddr = make_remote_address(argv);

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
        if (0 > send(s, send_buffer.c_str(), send_buffer.size(), 0))
        {
            cout <<"send failed\n";
            exit(1);
        }

        auto receive_buffer = string(BUFFESIZE, char(0));
        for (auto curr = receive_buffer.begin(); true; )
        {
            //RECEIVE
            if (0 >= recv(s, &*curr, 1, 0))
            {
                cout << "recv failed\n";
                exit(1);
            }
            if (*curr == '\n') 
            {  /*end on a LF*/
                receive_buffer = string(receive_buffer.begin(), curr);
                break;
            }
            if (*curr != '\r') ++curr;   /*ignore CR's*/
        }
        cout << receive_buffer << endl;
    }

    closesocket(s);
    return 0;
}