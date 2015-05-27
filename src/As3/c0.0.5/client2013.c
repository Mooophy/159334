/**
  *****************************************************************************
  * @file    client.c
  * @author  Yue Wang 12027710
  * @version V0.0.5
  * @date    11-Oct-2013
  * @brief   159.334 Assignment 3
   ****************************************************************************
  */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#define WSVERS MAKEWORD(2,0)
#define BUFFESIZE 200 
#define SEGMENTSIZE 70
//segment size, i.e., if fgets gets more than this number of bytes it segments the message

typedef struct Key
{
	int n;
	int e;
} Key;

WSADATA wsadata;
/* Prototype-------------------------------------------------------------------*/
int	repeatsquare( int x, int e, int n);
void	wy_key_builder(Key *_key, char *_buffer);
void	wy_RSA_encryptor(Key *_key, char *_buffer);

int main(int argc, char *argv[]) 
{
	Key key;
	
	
	//Initialization
   struct sockaddr_in remoteaddr;
   struct hostent *h;

   SOCKET s;

   char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
   int n,bytes;
   memset(&remoteaddr, 0, sizeof(remoteaddr)); //clean up 

	//WSASTARTUP 
   if (WSAStartup(WSVERS, &wsadata) != 0) 
	{
      WSACleanup();
      printf("WSAStartup failed\n");
   	exit(1);
   }
	
	//Dealing with user's arguments
   if (argc != 3) 
	{
      printf("USAGE: client IP-address port\n");
      exit(1);
   }
   else 
	{
	   remoteaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP address
	   remoteaddr.sin_port = htons((u_short)atoi(argv[2]));//Port number
   }
	//CREATE CLIENT'S SOCKET 
   s = socket(AF_INET, SOCK_STREAM, 0);
   if (s < 0) 
	{
      printf("socket failed\n");
   	exit(1);
   }
   remoteaddr.sin_family = AF_INET;
	//CONNECT
   if (connect(s, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr)) != 0) 
	{
      printf("connect failed\n");
   	exit(1);
   }
	
	
	/**
	  * @brief  receive public key.
	  */
	n = 0;
   while (1) 
	{
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
      printf("@alan: %s is received\n",receive_buffer);
		wy_key_builder(&key, receive_buffer);
		printf("@alan: key.n=%d, key.e=%d\n", key.n, key.e);
	
	
		
	
	//Get input while user don't type "."
   memset(&send_buffer, 0, BUFFESIZE);
   fgets(send_buffer,SEGMENTSIZE,stdin);
	
	while (strncmp(send_buffer,".",1) != 0) 
	{		
		printf("@alan before function m: %d %d %d\n", send_buffer[0], send_buffer[1],send_buffer[2]);
		printf("@alan before function e(m): %d %d %d\n", repeatsquare(send_buffer[0], key.e, key.n),repeatsquare(send_buffer[1], key.e, key.n),repeatsquare(send_buffer[2], key.e, key.n));
		wy_RSA_encryptor(&key, send_buffer);
		
      printf(">>> %s\n",send_buffer);//line sent
      strcat(send_buffer,"\r\n");

		//SEND
      bytes = send(s, send_buffer, strlen(send_buffer),0);
      if (bytes < 0) 
		{
         printf("send failed\n");
      	exit(1);
      }
      n = 0;
      while (1) 
		{
			//RECEIVE
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
      printf("%s \n",receive_buffer);// line received
      memset(&send_buffer, 0, BUFFESIZE);
      fgets(send_buffer,SEGMENTSIZE,stdin);
   }
	
	//CLOSESOCKET   
   closesocket(s);
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
	return y; //the result is stored in y
}

/**
  * @brief  build the key.
  */
void	wy_key_builder(Key *_key, char *_buffer)
{
	char bf[80];
	char sep[2] = " "; //separation is space
	char *word;
	int  wcount=0;
	
	strcpy(bf, _buffer);
	
	for (word = strtok(bf, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		
		if(wcount == 2) // jump the first space"_".
		{
			sscanf(word,"%d",&(_key->n));
		}
		
		if(wcount == 3) // jump the first space"_".
		{			
			sscanf(word,"%d",&(_key->e));			
		}
	}	
}
/**
  * @brief  encrpt and save it into the buffer passed in.
  */
void	wy_RSA_encryptor(Key *_key, char *_buffer)
{
	int temp[BUFFESIZE];	
	char Char_temp[BUFFESIZE];
	int i=0;
	int j=0;
	
	memset(temp, 0, BUFFESIZE);
	memset(Char_temp, 0, BUFFESIZE);
	
	while(*_buffer != 10)
	{		
		printf("@alan m=%d\n", *_buffer);
		temp[i] = repeatsquare(*_buffer, _key->e, _key->n);

		printf("@alan e(m)=%d\n",temp[i]);
		_buffer++;
		i++;
	}
	
	while(temp[j]!=0)
	{
		Char_temp[2*j] 		= 0x40	|	(temp[j]>>4);
		Char_temp[2*j +1]		= 0x40	|	(temp[j]& 0x0f);	
		j++;
	}
	printf("@alan the Char_temp=_%s_\n",Char_temp);
	
}
