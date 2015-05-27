/**
  *****************************************************************************
  * @file    client_rdt_YueWang_12027710.c
  * @author  Yue Wang 12027710
  * @version V1.0.0
  * @date    05-Oct-2013
  * @brief   1 Windows only.
  *			 2 Algorithm used here comes from the text book 6th edition Page 221.	
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
#define N_of_GBN 2
#define Timer_Limit 2

/* VARIABLES-------------------------------------------------------------------*/
typedef struct Timer
{
	int counting;
	int en;
} Timer;

WSADATA wsadata;
struct sockaddr_in localaddr,remoteaddr;
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.

/* Prototype-------------------------------------------------------------------*/
unsigned int CRCpolynomial(char *buffer);

unsigned int wy_CRC_check(char *_buffer);
int wy_get_ACK_SN(char *_buffer);
void wy_build_pkt(char *_data_buffer, int _next, char *_sndpkt);

/* MAIN------------------------------------------------------------------------*/
int main(int argc, char *argv[]) 
{
	int SN_base=0;
	int SN_next=0;
	Timer t;
	t.counting=0;
	t.en=1;
	char sndpkt[12][BUFFESIZE];
	
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

   char data_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
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
   FILE *fin=fopen("file1_Windows.txt","r");
   if(fin==NULL)
	{
	   printf("cannot open file\n");
	   exit(0);
   }
	
   while (1)
	{
		/**
		  * @brief  run the timer if enabled;
        */
		if(t.en)
		{
			t.counting++;	
		}
		
		memset(data_buffer, 0, sizeof(data_buffer));//clean up the send_buffer before reading the next line
		if(SN_next < (SN_base + N_of_GBN))
		{
			fgets(data_buffer,SEGMENTSIZE,fin);
		}
		
		if (!feof(fin)) 
		{
			/**
			  * @brief  Part A
		     * @note   send normally.
			  */
			if(SN_next < (SN_base + N_of_GBN))
			{
				wy_build_pkt(data_buffer, SN_next, sndpkt[SN_next]);//build
				send_unreliably(s,sndpkt[SN_next], remoteaddr);//send it
				
				if(SN_base == SN_next)
				{
					//start timer
					t.en=1;
				}
				SN_next++;				
			}
			/**
			  * @brief  Part B;
		     * @note   if timeout,send all packets not acknowledged.
		     */	
			if(t.counting > Timer_Limit)
			{
				t.en = 1; //start timer
				for(int i = SN_base;i < SN_next;i++ )
				{
					send_unreliably(s,sndpkt[i], remoteaddr);
					Sleep(1);
				}
			}
			Sleep(1000);
					
			/**
			  * @brief  Receive
		     * @note   Kept from the skeleton.
			  */
			memset(receive_buffer, 0, sizeof(receive_buffer));
			recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
			printf("RECEIVE -->%s\n",receive_buffer);
		
			/**
			  * @brief  Part C;
		     * @note   codes when CRC check passed.
		     */			  
			if(wy_CRC_check(receive_buffer))
			{
				if(wy_get_ACK_SN(receive_buffer)>-1)
				{
					SN_base = wy_get_ACK_SN(receive_buffer) + 1;	
				}
				
				if(SN_base == SN_next)
				{
					//stop timer
					t.en=0;
					t.counting=0;
				}
				else
				{
					//start timer
					t.en=1;
				}
			}
			Sleep(1000);
		}
		else 
		{
			printf("end of the file \n"); 
			memset(data_buffer, 0, sizeof(data_buffer)); 
			sprintf(data_buffer,"CLOSE \r\n");
			send_unreliably(s,data_buffer,remoteaddr);//we actually send this reliably, read UDP_supporting_functions_2012.c
		
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
  * @brief  Build packets and save it in _sndpkt.
  * @note   Using data,SN_next and CRC value.
  */
void wy_build_pkt(char *_data_buffer, int _next, char *_sndpkt)
{
	char data_buffer[BUFFESIZE];
	char temp_buffer[BUFFESIZE];
	char CRC_buffer[BUFFESIZE];
	
	memset(data_buffer, 0, sizeof(data_buffer));
	memset(temp_buffer, 0, sizeof(temp_buffer));
	memset( CRC_buffer, 0, sizeof (CRC_buffer));
	
	strcpy(data_buffer, _data_buffer);
	sprintf(temp_buffer, "PACKET %d ", _next);
	strcat(temp_buffer, _data_buffer);	//"SN data ""
	temp_buffer[strlen(temp_buffer)-1] = 0;
	sprintf(CRC_buffer, "%d ", CRCpolynomial(temp_buffer));
	strcat(CRC_buffer, temp_buffer);  	//"CRC SN data"

	strcat(CRC_buffer,"\n");	
	strcpy(_sndpkt, CRC_buffer);	
}
/**
  * @brief  return the Ack SN from the buffer specified;
  */
int wy_get_ACK_SN(char *_buffer)
{
	char bf[BUFFESIZE];	
	char sep[2] = " "; //separation is space
	char *word;
	int  wcount=0;
	int  sn=-10;
	
	strcpy(bf, _buffer);
	for (word = strtok(bf, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount == 3) // jump the first space"_".
		{
			sscanf(word,"%d",&sn);
		}	
	}	
	return sn;
}

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
	
	return CRC_client == CRCpolynomial(temp);
}

/**
  * @brief  return the generated CRC value.
  */
unsigned int CRCpolynomial(char *buffer)
{
	unsigned char i;
	unsigned int rem=0x0000;
				int bufsize=strlen((char*)buffer);
	while(bufsize--!=0)
	{
		for(i=0x80;i!=0;i/=2)
		{
			if((rem&0x8000)!=0)
			{
				rem=rem<<1;
				rem^=GENERATOR;
			}
     		else
			{
	   	   rem=rem<<1;
		   }
	  		if((*buffer&i)!=0)
			{
			   rem^=GENERATOR;
			}
		}
		buffer++;
	}
	rem=rem&0xffff;
	return rem;
}
