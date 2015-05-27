//c0.0.5

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "UDP_supporting_functions_2013.c"


#define WSVERS MAKEWORD(2,0)
#define BUFFESIZE 80 
#define SEGMENTSIZE 78
#define GENERATOR 0x8005 //0x8005, generator for polynomial division
#define N_of_GBN 2
#define Timer_Limit 2


class Timer
{
	protected:
		int en;
		int counting;
	
	public:
		Timer(void)
		{
			this->en	=	1;
			this->counting	=	0;
		}
		
		Timer(int _e,	int _c)
		{
			this->en	=	_e;
			this->counting	= _c;
		}
		
		const	int getState(void)
		{
			return this->en;
		}
		const	int getCounting(void)
		{
			return this->counting;
		}
		
		void	increment(void)
		{
			this->counting++;
		}
		
		void	start(void)
		{
			this->en	=	1;
		}
		
		void	stop(void)
		{
			this->en=0;
		}
		
		void	clear(void)
		{
			this->counting	=	0;
		}
};

WSADATA wsadata;
struct sockaddr_in localaddr,remoteaddr;
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.


unsigned int CRCpolynomial(char *buffer);

unsigned int CRC_check(char *_buffer);
int get_ACK_SN(char *_buffer);
void build_pkt(char *_data_buffer, int _next, char *_sndpkt);


int main(int argc, char *argv[]) 
{
	int SN_base=0;
	int SN_next=0;

	Timer t(1,0);
	
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
		if(t.getState()==1)
		{
			t.increment();	
		}
		
		memset(data_buffer, 0, sizeof(data_buffer));//clean up the send_buffer before reading the next line
		if(SN_next < (SN_base + N_of_GBN))
		{
			fgets(data_buffer,SEGMENTSIZE,fin);
		}
		
		if (!feof(fin)) 
		{
			if(SN_next < (SN_base + N_of_GBN))
			{
				build_pkt(data_buffer, SN_next, sndpkt[SN_next]);//build
				send_unreliably(s,sndpkt[SN_next], remoteaddr);//send it
				
				if(SN_base == SN_next)
				{
					t.start();
				}
				SN_next++;				
			}

			if(t.getCounting() > Timer_Limit)
			{
				t.start();
				for(int i = SN_base;i < SN_next;i++ )
				{
					send_unreliably(s,sndpkt[i], remoteaddr);
					Sleep(1);
				}
			}
			Sleep(1000);
					

			memset(receive_buffer, 0, sizeof(receive_buffer));
			recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
			printf("RECEIVE -->%s\n",receive_buffer);
		
		  
			if(CRC_check(receive_buffer))
			{
				if(get_ACK_SN(receive_buffer)>-1)
				{
					SN_base = get_ACK_SN(receive_buffer) + 1;	
				}
				
				if(SN_base == SN_next)
				{
					t.stop();
					t.clear();
				}
				else
				{
					t.start();
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


void build_pkt(char *_data_buffer, int _next, char *_sndpkt)
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

int get_ACK_SN(char *_buffer)
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


unsigned int CRC_check(char *_buffer)
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
