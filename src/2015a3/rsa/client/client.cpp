#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#include <string>
#include <iostream>
using std::cout;
using std::endl;

#define WSVERS MAKEWORD(2,0)
#define BUFFESIZE 200 
#define SEGMENTSIZE 70

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
        printf("WSAStartup failed\n");
        exit(1);
    }
    return wsadata;
}


auto main(int argc, char *argv[]) -> int
{
    handle_user_input(argc);
    auto wsa_data = setup_win_sock_api(WSVERS);
    auto remoteaddr = make_remote_address(argv);

    char send_buffer[BUFFESIZE], receive_buffer[BUFFESIZE];
    int n, bytes;

    //CREATE CLIENT'S SOCKET 
    auto s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        printf("socket failed\n");
        exit(1);
    }

    //CONNECT
    if (connect(s, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr)) != 0) 
    {
        printf("connect failed\n");
        exit(1);
    }

    //Get input while user don't type "."
    memset(&send_buffer, 0, BUFFESIZE);
    fgets(send_buffer, SEGMENTSIZE, stdin);
    while (strncmp(send_buffer, ".", 1) != 0)
    {
        send_buffer[strlen(send_buffer) - 1] = '\0';//strip send_buffer from '\n'
        strcat(send_buffer, "\r\n");
        //*******************************************************************
        //SEND
        //*******************************************************************
        bytes = send(s, send_buffer, strlen(send_buffer), 0);
        if (bytes < 0) {
            printf("send failed\n");
            exit(1);
        }
        n = 0;
        while (1) 
        {
            //*******************************************************************
            //RECEIVE
            //*******************************************************************
            bytes = recv(s, &receive_buffer[n], 1, 0);
            if ((bytes <= 0)) 
            {
                printf("recv failed\n");
                exit(1);
            }
            if (receive_buffer[n] == '\n') 
            {  /*end on a LF*/
                receive_buffer[n] = '\0';
                break;
            }
            if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
        }
        printf("%s \n", receive_buffer);// line received
        memset(&send_buffer, 0, BUFFESIZE);
        fgets(send_buffer, SEGMENTSIZE, stdin);
    }

    closesocket(s);
    return 0;
}