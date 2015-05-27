#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include "UDP_supporting_functions_2013.c"


#define BUFFESIZE 80 
#define SEGMENTSIZE 78
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.
#define WSVERS MAKEWORD(2,0)
#define NUMBER_OF_WORDS_IN_THE_HEADER 3 // used in save_line_without_header.
#define GENERATOR 0x8005 //0x8005, generator for polynomial division


WSADATA wsadata;

void save_line_without_header(char * receive_buffer,FILE *fout);
unsigned int CRCpolynomial(char *buffer);

unsigned int SN_check(int _SN_expected, char *_buffer);
unsigned int CRC_check(char *_buffer);
void			 ACK_build(char *_buffer, int _sn);


int main(int argc, char *argv[]) 
{	
	//Initialization
	struct sockaddr_in localaddr,remoteaddr;
 
	SOCKET s;
	char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
	int n,bytes,addrlen;
	
	int SN_expected = 0;		//expecting seq 0
	int SN_checked = -1;		//no seq is checked
	
	addrlen = sizeof(remoteaddr);
	memset(&localaddr,0,sizeof(localaddr));//clean up the structure
	memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure
	randominit();

	// WSSTARTUP
	if (WSAStartup(WSVERS, &wsadata) != 0) 
	{
		WSACleanup();
		printf("WSAStartup failed\n");
	}

	//SOCKET
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s <0) 
		{
		printf("socket failed\n");
	}
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
	if (argc != 4) 
	{
		printf("2012 code USAGE: server port  lossesbit(0 or 1) damagebit(0 or 1)\n");
		exit(1);
	}
	localaddr.sin_port = htons((u_short)atoi(argv[1]));
	int remotePort=1234;
	packets_damagedbit=atoi(argv[3]);
	packets_lostbit=atoi(argv[2]);
	if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1)
	{
		printf("2012 code USAGE: server port  lossesbit(0 or 1) damagebit(0 or 1)\n");
		exit(0);
	}

	//REMOTE HOST IP AND PORT
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteaddr.sin_port = htons(remotePort);


	//BIND
	if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) 
	{
		printf("Bind failed!\n");
		exit(0);
	}
	FILE *fout=fopen("file1_saved.txt","w");// Open file to save the incoming packets
	
	
	while (1) 
	{
		//RECEIVE
		bytes = recvfrom(s, receive_buffer, SEGMENTSIZE, 0,(struct sockaddr *)(&remoteaddr),&addrlen);

		//PROCESS REQUEST
		n=0;
		while (n<bytes)
		{
			n++;
			if ((bytes < 0) || (bytes == 0)) break;
			if (receive_buffer[n] == '\n') /*end on a LF*/
			{ 
				receive_buffer[n] = '\0';
				break;
			}
			if (receive_buffer[n] == '\r') /*ignore CRs*/
				receive_buffer[n] = '\0';
		}
		if ((bytes < 0) || (bytes == 0)) break;
		printf("\n================================================\n");	
		printf("RECEIVED --> %s \n",receive_buffer);		
				

	
		//"close" handling
		if (strncmp(receive_buffer,"CLOSE",5)==0)  
		{
			fclose(fout);
			closesocket(s);
			printf("Server saved file1_saved.txt \n");
			exit(0);
		}
		

		if(SN_check(SN_expected,receive_buffer)&& CRC_check(receive_buffer))	//check if received pack is expected and if the packect is completed
		{
			save_line_without_header(receive_buffer,fout); //save without header.
			
			SN_expected++;
			SN_checked ++;
		}
		

		if(SN_checked >= 0)
		{
			ACK_build(send_buffer, SN_checked);		//save check seq number
			send_unreliably(s,send_buffer,remoteaddr);	//send it to client
		}
	
	}
	closesocket(s);
	exit(0);
}



void ACK_build(char *_buffer, int _sn)		//build ack packet
{
	char CRC_buffer[10],temp_buffer[BUFFESIZE];
	
	sprintf(temp_buffer,"ACKNOW %d", _sn);
	sprintf(CRC_buffer, "%d", CRCpolynomial(temp_buffer));
	strcat(CRC_buffer," ");
	strcat(CRC_buffer,temp_buffer);
	strcpy(temp_buffer,CRC_buffer);
	strcat(temp_buffer," \r\n");
	strcpy(_buffer, temp_buffer);
}

unsigned int CRC_check(char *_buffer)		//check if recieved message is completed
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

unsigned int SN_check(int _SN_expected, char *_buffer)		//check if received seq is expected
{
	char bf[BUFFESIZE];
	char sep[2] = " "; //separation is space
	char *word;
	int  wcount=0;
	int  SN_extracted;
	
	strcpy(bf, _buffer);
	
	for (word = strtok(bf, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount == 3) 
		{
			sscanf(word,"%d",&SN_extracted);
		}	
	}
	return _SN_expected == SN_extracted;	
}

void save_line_without_header(char * receive_buffer,FILE *fout){	
	char sep[2] = " "; //separation is space
	char *word;
	int wcount=0;
	char lineoffile[BUFFESIZE]="\0";
	for (word = strtok(receive_buffer, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount > NUMBER_OF_WORDS_IN_THE_HEADER){
			strcat(lineoffile,word);
			strcat(lineoffile," ");
		}	
	}
	printf("DATA: %s \n",lineoffile);
	lineoffile[strlen(lineoffile)-1]=(char)'\0';//get rid of last space
	if (fout!=NULL) fprintf(fout,"%s\n",lineoffile);
	else 
	{
		printf("error when trying to write...\n");
		exit(0);
	}	
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
