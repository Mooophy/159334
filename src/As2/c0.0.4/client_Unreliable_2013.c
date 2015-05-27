/**
  *****************************************************************************
  * @file    client.c
  * @author  Yue Wang 12027710
  * @version V0.0.4
  * @date    03-Oct-2013
  * @brief   
  ******************************************************************************
  *
  */
/* Includes ------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "UDP_supporting_functions_2013.c"
/* MACRO ---------------------------------------------------------------------*/
#define WSVERS MAKEWORD(2,0)
#define BUFFESIZE 80 
#define SEGMENTSIZE 78
#define GENERATOR 0x8005 //0x8005, generator for polynomial division

/* VARIABLES-------------------------------------------------------------------*/
WSADATA wsadata;
struct sockaddr_in localaddr,remoteaddr;
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.

/* Prototype-------------------------------------------------------------------*/
unsigned int CRCpolynomial(char *buffer);
unsigned int wy_CRC_check(char *_buffer);

/* MAIN------------------------------------------------------------------------*/
int main(int argc, char *argv[]) 
{
   // WSSTARTUP
	if (WSAStartup(WSVERS, &wsadata) != 0) 
	{
		WSACleanup();
		printf("WSAStartup failed\n");
	}

	// Initialization
	memset(&localaddr, 0, sizeof(localaddr));//clean up
   int localPort=1234;
   localaddr.sin_family = AF_INET;
   localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
   localaddr.sin_port = htons(localPort);
   memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up  
   randominit();

   SOCKET s;

   char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
   remoteaddr.sin_family = AF_INET;

	//	Dealing with user's arguments
   if (argc != 5) 
	{
	   printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
      exit(1);
   }
   remoteaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP address
   remoteaddr.sin_port = htons((u_short)atoi(argv[2]));//always get the port number
   packets_lostbit=atoi(argv[3]);
   packets_damagedbit=atoi(argv[4]);
   if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1)
	{
	   printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
	   exit(0);
   }
	

	//CREATE CLIENT'S SOCKET 
   s = socket(AF_INET, SOCK_DGRAM, 0);//this is a UDP socket
   if (s < 0) 
	{
      printf("socket failed\n");
   	exit(1);
   }
	
   //NONBLOCKING OPTION for Windows
   u_long iMode=1;
   ioctlsocket(s,FIONBIO,&iMode);


	//Open A TEXT FILE 
   int counter=0;//sequence of packets
   int ackcounter=0;
   char temp_buffer[BUFFESIZE];
	char CRC_str[BUFFESIZE];

   FILE *fin=fopen("file1_Windows.txt","r");
   if(fin==NULL)
	{
	   printf("cannot open file\n");
	   exit(0);
   }
	
   while (1)
	{
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
		//----------------------
		
		send_unreliably(s,send_buffer, remoteaddr);		

		Sleep(1000);
		

		//RECEIVE
		memset(receive_buffer, 0, sizeof(receive_buffer));
		recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
		printf("RECEIVE -->%s__\n",receive_buffer);
		
		
		
		
		//printf("@Alan the CRC check result is %d\n",wy_CRC_check(receive_buffer));
		
		
		

		Sleep(1000);
	}
	else 
	{
		printf("end of the file \n"); 
		memset(send_buffer, 0, sizeof(send_buffer)); 
		sprintf(send_buffer,"CLOSE \r\n");
		send_unreliably(s,send_buffer,remoteaddr);//we actually send this reliably, read UDP_supporting_functions_2012.c
		
		break;
	}
   }

	//CLOSESOCKET   
   printf("closing everything on the client's side ... \n");
   fclose(fin);

   closesocket(s);
   exit(0);
}


/* Function definitions  ------------------------------------------------------*/

/**
  * @brief  check the CRC value, return 1 if pass, otherwise 0;
  */
unsigned int wy_CRC_check(char *_buffer)
{
	char bf[BUFFESIZE];
	char temp[BUFFESIZE]="\0";
	char sep[2] = " "; //separation is space
	char *word;
	int  wcount=0;
	unsigned int  CRC_client=0;
	
	strcpy(bf, _buffer);
	sscanf(bf, "%d", &CRC_client);

	for (word = strtok(bf, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount > 1) // jump the first space"_".
		{
			strcat(temp,word);
			strcat(temp," ");
		}	
	}
	temp[strlen(temp)-1] = 0;// delete the last space .
	
	//printf("test: CRC is %d     the sting is %s  the first is :%d",CRCpolynomial(temp),temp,temp[0]);
	//printf("test: the result is %d,",CRC_client == CRCpolynomial(temp));
	return CRC_client == CRCpolynomial(temp);
}

/**
  * @brief  return the generated CRC value.
  */
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
