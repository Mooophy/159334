#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#define WSVERS MAKEWORD(2,0)

#define BUFFESIZE 200 
#define SEGMENTSIZE 70

WSADATA wsadata;

int main(int argc, char *argv[]) 
{
    //*******************************************************************
    // Initialization
    //*******************************************************************
    struct sockaddr_in remoteaddr;
    struct hostent *h;

    SOCKET s;
    char send_buffer[BUFFESIZE], receive_buffer[BUFFESIZE];
    int n, bytes;
    memset(&remoteaddr, 0, sizeof(remoteaddr)); //clean up 
    //*******************************************************************
    //WSASTARTUP 
    //*******************************************************************
    if (WSAStartup(WSVERS, &wsadata) != 0) {
        WSACleanup();
        printf("WSAStartup failed\n");
        exit(1);
    }

    if (argc != 3) {
        printf("USAGE: client IP-address port\n");
        exit(1);
    }
    else {
        remoteaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP address
        remoteaddr.sin_port = htons((u_short)atoi(argv[2]));//Port number
    }
    //*******************************************************************
    //CREATE CLIENT'S SOCKET 
    //*******************************************************************
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("socket failed\n");
        exit(1);
    }
    remoteaddr.sin_family = AF_INET;
    //*******************************************************************
    //CONNECT
    //*******************************************************************
    if (connect(s, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr)) != 0) {
        printf("connect failed\n");
        exit(1);
    }
    //*******************************************************************
    //Get input while user don't type "."
    //*******************************************************************
    memset(&send_buffer, 0, BUFFESIZE);
    fgets(send_buffer, SEGMENTSIZE, stdin);
    while (strncmp(send_buffer, ".", 1) != 0) {
#if defined __unix__ || defined __APPLE__
        send_buffer[strlen(send_buffer) - 1] = '\0';//strip send_buffer from '\n'
        printf("lenght is %d \n", (int)strlen(send_buffer));
#elif defined _WIN32 
        send_buffer[strlen(send_buffer) - 1] = '\0';//strip send_buffer from '\n'
        //nothing to do
#endif
        printf(">>> %s\n", send_buffer);//line sent
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
        while (1) {
            //*******************************************************************
            //RECEIVE
            //*******************************************************************
            bytes = recv(s, &receive_buffer[n], 1, 0);
            if ((bytes <= 0)) {
                printf("recv failed\n");
                exit(1);
            }
            if (receive_buffer[n] == '\n') {  /*end on a LF*/
                receive_buffer[n] = '\0';
                break;
            }
            if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
        }
        printf("%s \n", receive_buffer);// line received
        memset(&send_buffer, 0, BUFFESIZE);
        fgets(send_buffer, SEGMENTSIZE, stdin);
    }
    //*******************************************************************
    //CLOSESOCKET   
    //*******************************************************************
#if defined __unix__ || defined __APPLE__
    close(s);
#elif defined _WIN32 
    closesocket(s);
#endif

    return 0;
}
