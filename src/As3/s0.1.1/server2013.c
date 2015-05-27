/**
  *****************************************************************************
  * @file    server.c
  * @author  Yue Wang 12027710
  * @version V0.1.1
  * @date    11-Oct-2013
  * @brief   159.334 Assignment 3
   ****************************************************************************
  */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;

#define BUFFESIZE 200
#define SEGMENTSIZE 198
//segment size, i.e., BUFFESIZE - 2 bytes (for \r\n)

/* Prototype-------------------------------------------------------------------*/
int repeatsquare( int x, int e, int n); 
void decryptor(char *_buffer, int _n, int _d);

int main(int argc, char *argv[]) 
{
	//@alan {n,e,d}
	int Keys[3][3]={{143, 7, 103}, {187, 27, 83}, {209, 17, 53}};
	int Key_counter = 0;
	
	//INITIALIZATION
   struct sockaddr_in localaddr,remoteaddr;

   SOCKET s, ns;

   char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
   memset(&send_buffer,0,BUFFESIZE);
   memset(&receive_buffer,0,BUFFESIZE);

   int n,bytes,addrlen;
   memset(&localaddr,0,sizeof(localaddr));//clean up the structure
   memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure

	//WSSTARTUP
   if (WSAStartup(WSVERS, &wsadata) != 0) 
	{
      WSACleanup();
      printf("WSAStartup failed\n");
   }

	//SOCKET
   s = socket(PF_INET, SOCK_STREAM, 0);
   if (s < 0) 
	{
      printf("socket failed\n");
   }

   localaddr.sin_family = AF_INET;
   if (argc == 2) localaddr.sin_port = htons((u_short)atoi(argv[1]));
   else localaddr.sin_port = htons(1234);//default listening port
   localaddr.sin_addr.s_addr = INADDR_ANY;//server IP address should be local

	//BIND
   if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) 
	{
      printf("Bind failed!\n");
      exit(0);
   }
	
	//LISTEN
   listen(s,5);

   while (1) 
	{
      addrlen = sizeof(remoteaddr);

		//NEW SOCKET newsocket = accept
      ns = accept(s,(struct sockaddr *)(&remoteaddr),&addrlen);

      if (ns < 0) break;
      printf("accepted a connection from client IP %s port %d \n",inet_ntoa(remoteaddr.sin_addr),ntohs(localaddr.sin_port));
		
		/**
		  * @brief  send public key.
		  */
		if(Key_counter>2)
		{
			Key_counter=0;
		}		
		sprintf(send_buffer, "Public_Key %d %d\r\n",Keys[Key_counter][0],Keys[Key_counter][1]);
		
		bytes = send(ns, send_buffer, strlen(send_buffer), 0);
		printf("@alan: %s", send_buffer);
		
      while (1) 
		{
         n = 0;
         while (1) 
			{
				//RECEIVE
            bytes = recv(ns, &receive_buffer[n], 1, 0);

				//PROCESS REQUEST
            if ((bytes <=0 )) break;
            if (receive_buffer[n] == '\n') 
				{ /*end on a LF*/
               receive_buffer[n] = '\0';
               break;
            }
            if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
         }
         if (bytes <= 0) break;
				
			printf("The client is sending: %s\n",receive_buffer);
			memset(&send_buffer, 0, BUFFESIZE);
			sprintf(send_buffer, "<<< SERVER SAYS:The client typed '%s' - There are %d bytes of information\r\n", receive_buffer, n);
			
			
			
			
			/**
			  * @brief  decrypt and save it in the receive_buffer
			  */
			decryptor(receive_buffer, Keys[Key_counter][0], Keys[Key_counter][2]);
			printf("@alan the decrpted message is:'%s'\n",receive_buffer);
			printf("...................................\n");
			
			
			
			
			//SEND
         bytes = send(ns, send_buffer, strlen(send_buffer), 0);
         if (bytes < 0) break;
      }
			//CLOSE SOCKET
      closesocket(ns);//close connecting socket
      printf("disconnected from %s\n",inet_ntoa(remoteaddr.sin_addr));
		
		
		/**
		  * @brief  next key
		  */
		Key_counter++;		
   }

   closesocket(s);//close listening socket
   return 0;
}

/* Function definitions  ------------------------------------------------------*/
/**
  * @brief  return the value of (x^e)%n.
  */
int repeatsquare( int x, int e, int n) 
{
	int y=1;//initialize y to 1, very important
	while (e > 0) 
	{
		if (( e % 2 ) == 0) 
		{
			x = (x*x) % n;
			e = e/2;
		}
		else 
		{
			y = (x*y) % n;
			e = e-1;
		}
}
	return y; 
}

/**
  * @brief decrypt the msg and save it to the buffer passed in
  */
void decryptor(char *_buffer, int _n, int _d)
{
	unsigned int i=0, j=0;
	char temp[BUFFESIZE];
	int build_temp[BUFFESIZE];
	char de_temp[BUFFESIZE];
	
	memset(temp, 0, BUFFESIZE);
	memset(build_temp, 0, BUFFESIZE);
	memset(de_temp, 0, BUFFESIZE);	
	
	strcpy(temp, _buffer);
	while(2*i < strlen(temp))
	{
		build_temp[i]	=	((temp[2*i] & 0x0f)<<4)	|	(temp[2*i+1]	&	0x0f);
		i++;
	}
	
	while(build_temp[j] != 0)
	{
		de_temp[j] = repeatsquare(build_temp[j], _d, _n);
		j++;
	}
	strcpy(_buffer, de_temp);
}
