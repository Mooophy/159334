#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "../lib/as3.hpp"
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin;

#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;

#define BUFFESIZE 200
#define SEGMENTSIZE 198
//segment size, i.e., BUFFESIZE - 2 bytes (for \r\n)

int main(int argc, char *argv[]) 
{

    //********************************************************************
    // INITIALIZATION
    //********************************************************************
    sockaddr_in localaddr, remoteaddr;

    SOCKET ns;
    char send_buffer[BUFFESIZE], receive_buffer[BUFFESIZE];
    memset(&send_buffer, 0, BUFFESIZE);
    memset(&receive_buffer, 0, BUFFESIZE);

    int n, bytes, addrlen;
    memset(&localaddr, 0, sizeof(localaddr));//clean up the structure
    memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up the structure

    //********************************************************************
    // WSSTARTUP
    //********************************************************************
    if (WSAStartup(WSVERS, &wsadata) != 0) 
    {
        WSACleanup();
        printf("WSAStartup failed\n");
    }
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
        //********************************************************************
        //NEW SOCKET newsocket = accept
        //********************************************************************
        as3::Socket new_sock{ accept(sock.get(), (sockaddr*)(&remoteaddr), &addrlen) };
        if ( new_sock.is_failed()) break;
        printf("accepted a connection from client IP %s port %d \n", inet_ntoa(remoteaddr.sin_addr), ntohs(localaddr.sin_port));
        while (1) 
        {
            n = 0;
            while (1) 
            {
                //********************************************************************
                //RECEIVE
                //********************************************************************
                bytes = recv(new_sock.get(), &receive_buffer[n], 1, 0);
                //********************************************************************
                //PROCESS REQUEST
                //********************************************************************
                if (bytes <= 0) break;
                if (receive_buffer[n] == '\n') { /*end on a LF*/
                    receive_buffer[n] = '\0';
                    break;
                }
                if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
            }
            if (bytes <= 0) break;
            printf("The client is sending: %s\n", receive_buffer);
            memset(&send_buffer, 0, BUFFESIZE);
            sprintf(send_buffer, "<<< SERVER SAYS:The client typed '%s' - There are %d bytes of information\r\n", receive_buffer, n);

            //********************************************************************
            //SEND
            //********************************************************************
            bytes = send(new_sock.get(), send_buffer, strlen(send_buffer), 0);
            if (bytes < 0) break;
        }
        //********************************************************************
        //CLOSE SOCKET
        //********************************************************************
        printf("disconnected from %s\n", inet_ntoa(remoteaddr.sin_addr));
    }
    return 0;
}