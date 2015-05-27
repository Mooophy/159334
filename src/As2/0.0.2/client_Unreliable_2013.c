/**
  *****************************************************************************
  * @file    client.c
  * @author  Yue Wang 12027710
  * @version V0.0.2
  * @date    03-Oct-2013
  * @brief   
   ****************************************************************************
  * @history 0.0.1 function added : unsigned int wy_CRC_check(char *_buffer).	
  *			 0.0.2 nothing.
  */
//159.334 - Networks
// CLIENT: prototype for assignment 2. 
///////////////   2013 ////////////////////
// This code is different than the one used in previous semesters...
//************************************************************************/
//COMPILE WITH: gcc client_Unreliable_2013.c -o server_Unreliable_2013
//with no losses nor damages, RUN WITH: ./client_Unreliable_2013 127.0.0.1 1234 0 0 
//with losses RUN WITH: ./client_Unreliable_2013 127.0.0.1 1234 1 0 
//with damages RUN WITH: ./client_Unreliable_2013 127.0.0.1 1234 0 1 
//with losses and damages RUN WITH: ./client_Unreliable_2013 127.0.0.1 1234 1 1  
//************************************************************************/
#if defined __unix__ || defined __APPLE__
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <netdb.h>
#elif defined _WIN32 
# include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;
#endif

#include "UDP_supporting_functions_2013.c"
#define BUFFESIZE 80 
//rememver, the BUFFESIZE has to be at least big enough to receive the answer from the server
#define SEGMENTSIZE 78
#define GENERATOR 0x8005 //0x8005, generator for polynomial division
struct sockaddr_in localaddr,remoteaddr;
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.

//********************************************************************
// PROTOTYPE
//********************************************************************
unsigned int CRCpolynomial(char *buffer);
// Simple usage: client IP port, or client IP (use default port) 
int main(int argc, char *argv[]) {
#if defined __unix__ || defined __APPLE__
	
#elif defined _WIN32 
//********************************************************************
// WSSTARTUP
//********************************************************************
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}
#endif
//*******************************************************************
// Initialization
//*******************************************************************
   memset(&localaddr, 0, sizeof(localaddr));//clean up
   //char localIP[INET_ADDRSTRLEN]="127.0.0.1";
   int localPort=1234;
   localaddr.sin_family = AF_INET;
   //localaddr.sin_addr.s_addr = inet_addr(localIP);
   localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
//********************************************************************
   localaddr.sin_port = htons(localPort);
   memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up  
   randominit();
#if defined __unix__ || defined __APPLE__
   int s;
#elif defined _WIN32
   SOCKET s;
#endif
   char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
   remoteaddr.sin_family = AF_INET;
//*******************************************************************
//	Dealing with user's arguments
//*******************************************************************
   if (argc != 5) {
	   printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
      exit(1);
   }
   remoteaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP address
   remoteaddr.sin_port = htons((u_short)atoi(argv[2]));//always get the port number
   //localaddr.sin_port = htons((u_short)atoi(argv[3]));//always get the port number
   packets_lostbit=atoi(argv[3]);
   packets_damagedbit=atoi(argv[4]);
   if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1){
	   printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
	   exit(0);
   }
//*******************************************************************
//CREATE CLIENT'S SOCKET 
//*******************************************************************
   s = socket(AF_INET, SOCK_DGRAM, 0);//this is a UDP socket
   if (s < 0) {
      printf("socket failed\n");
   	exit(1);
   }
#if defined __unix__ || defined __APPLE__

#elif defined _WIN32
   //***************************************************************//
   //NONBLOCKING OPTION for Windows
   //***************************************************************//
   u_long iMode=1;
   ioctlsocket(s,FIONBIO,&iMode);
#endif
//*******************************************************************
//SEND A TEXT FILE 
//*******************************************************************
   int counter=0;//sequence of packets
   int ackcounter=0;
   char temp_buffer[BUFFESIZE];
	char CRC_str[BUFFESIZE];
#if defined __unix__ || defined __APPLE__
   FILE *fin=fopen("file1.txt","r");
#elif defined _WIN32
   FILE *fin=fopen("file1_Windows.txt","r");
#endif
   if(fin==NULL){
	   printf("cannot open file\n");
	   exit(0);
   }
   while (1){
	memset(send_buffer, 0, sizeof(send_buffer));//clean up the send_buffer before reading the next line
	fgets(send_buffer,SEGMENTSIZE,fin);
	if (!feof(fin)) {
		
		//add a headert to the packet with the sequence number
		sprintf(temp_buffer,"PACKET %d ",counter);
		counter++;
		strcat(temp_buffer,send_buffer);
		strcpy(send_buffer,temp_buffer);
		
		//---------coding-------
		temp_buffer[strlen(temp_buffer)-1] = 0;
		sprintf(CRC_str,"%d ",CRCpolynomial(temp_buffer));
		strcat(CRC_str,send_buffer);
		strcpy(send_buffer, CRC_str);
		//printf("testing:the CRC is %d,lenth is %d \n",CRCpolynomial(temp_buffer),strlen(temp_buffer));
		//printf("len= %d    the last four :%d %d %d %d",strlen(temp_buffer),temp_buffer[34],temp_buffer[35],temp_buffer[36],temp_buffer[37]);
		//----------------------
		
		send_unreliably(s,send_buffer, remoteaddr);
		
#if defined __unix__ || defined __APPLE__
		sleep(1);
#elif defined _WIN32
		Sleep(1000);
#endif
	//********************************************************************
	//RECEIVE
	//********************************************************************
		memset(receive_buffer, 0, sizeof(receive_buffer));
		recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
		printf("RECEIVE --> %s \n",receive_buffer);
#if defined __unix__ || defined __APPLE__
		sleep(1);//wait for a bit before trying the next packet
#elif defined _WIN32
		Sleep(1000);
#endif
	}
	else {
		printf("end of the file \n"); 
		memset(send_buffer, 0, sizeof(send_buffer)); 
		sprintf(send_buffer,"CLOSE \r\n");
		send_unreliably(s,send_buffer,remoteaddr);//we actually send this reliably, read UDP_supporting_functions_2012.c
		
		break;
	}
   }
//*******************************************************************
//CLOSESOCKET   
//*******************************************************************
   printf("closing everything on the client's side ... \n");
   fclose(fin);
#if defined __unix__ || defined __APPLE__
   close(s);
#elif defined _WIN32
   closesocket(s);
#endif

   exit(0);

}
//*******************************************************************
//FUNCTIONS 
//*******************************************************************
unsigned int CRCpolynomial(char *buffer){
	unsigned char i;
	unsigned int rem=0x0000;
        int bufsize=strlen((char*)buffer);
	while(bufsize--!=0){
		for(i=0x80;i!=0;i/=2){
			if((rem&0x8000)!=0){
				rem=rem<<1;
				rem^=GENERATOR;
			}
     		else{
	   	   rem=rem<<1;
		   }
	  		if((*buffer&i)!=0){
			   rem^=GENERATOR;
			}
		}
		buffer++;
	}
	rem=rem&0xffff;
	return rem;
}
