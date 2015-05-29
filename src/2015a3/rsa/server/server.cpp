#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "../lib/as3.hpp"
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

#define BUFFESIZE 200
#define SEGMENTSIZE 198
//segment size, i.e., BUFFESIZE - 2 bytes (for \r\n)

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
        printf("Bind failed!\n");
        exit(0);
    }
    //********************************************************************
    //LISTEN
    //********************************************************************
    listen(sock.get(), 5);

    //********************************************************************
    //INFINITE LOOP
    //********************************************************************
    while (1)
    {
        addrlen = sizeof(remoteaddr);
        as3::Socket new_sock{ ::accept(sock.get(), (sockaddr*)(&remoteaddr), &addrlen) };
        if (new_sock.is_failed()) break;
        printf("accepted a connection from client IP %s port %d \n", inet_ntoa(remoteaddr.sin_addr), ntohs(localaddr.sin_port));
        while (1)
        {
            auto message_reveived = as3::receive(new_sock.get());
            //strcpy(receive_buffer, message_reveived.c_str());
            cout << "The client is sending:\n" << message_reveived << endl;
            //memset(&send_buffer, 0, BUFFESIZE);
            auto feed_back = "<<< SERVER SAYS:The client typed '" + message_reveived + "' -- " + std::to_string(message_reveived.size()) + " bytes in total\r\n";
            //sprintf(send_buffer, "<<< SERVER SAYS:The client typed '%s' - There are %d bytes of information\r\n", receive_buffer, message_reveived.size());
            as3::send(new_sock.get(), feed_back);
        }
        cout << "disconnected from " << inet_ntoa(remoteaddr.sin_addr) << endl;
    }
    return 0;
}