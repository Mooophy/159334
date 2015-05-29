#include <stdlib.h>
#include <winsock.h>
#include "../lib/as3.hpp"
#include <string>
using std::string; using std::to_string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

#define BUFFESIZE 200
#define SEGMENTSIZE 198

int main(int argc, char *argv[])
{

    //********************************************************************
    // INITIALIZATION
    //********************************************************************
    sockaddr_in localaddr, remoteaddr;

    char send_buffer[BUFFESIZE], receive_buffer[BUFFESIZE];
    memset(&send_buffer, 0, BUFFESIZE);
    memset(&receive_buffer, 0, BUFFESIZE);

    int bytes, addrlen;
    memset(&localaddr, 0, sizeof(localaddr));//clean up the structure
    memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up the structure

    as3::setup_win_sock_api(as3::WSVERS);
    //********************************************************************
    //SOCKET
    //********************************************************************
    as3::Socket sock{ AF_INET, SOCK_STREAM, 0 };
    if (sock.is_failed()) cout << "socket failed\n";

    localaddr.sin_family = AF_INET;
    if (argc == 2) localaddr.sin_port = htons((u_short)atoi(argv[1]));
    else localaddr.sin_port = htons(1234);//default listening port
    localaddr.sin_addr.s_addr = INADDR_ANY;//server IP address should be local
    //********************************************************************
    //BIND
    //********************************************************************
    if (bind(sock.get(), (struct sockaddr *)(&localaddr), sizeof(localaddr)) != 0)
    {
        as3::println("Bind failed!");
        exit(0);
    }
    //********************************************************************
    //LISTEN
    //********************************************************************
    listen(sock.get(), 5);
    while (1)
    {
        addrlen = sizeof(remoteaddr);
        as3::Socket new_sock{ ::accept(sock.get(), (sockaddr*)(&remoteaddr), &addrlen) };
        if (new_sock.is_failed()) break;
        as3::println("accepted a connection from client IP " + string(inet_ntoa(remoteaddr.sin_addr)) + " port " + to_string(ntohs(localaddr.sin_port)));
        for (auto receive = as3::Receive{}; receive.is_normal(); )
        {
            auto message_reveived = receive(new_sock);
            as3::println("The client is sending:\n" + message_reveived);
            auto feed_back = "<<< SERVER SAYS:The client typed '" + message_reveived + "' -- " + to_string(message_reveived.size()) + " bytes in total\r\n";
            as3::send(new_sock.get(), feed_back);
        }
        as3::println("disconnected from " + string(inet_ntoa(remoteaddr.sin_addr)));
    }
    return 0;
}