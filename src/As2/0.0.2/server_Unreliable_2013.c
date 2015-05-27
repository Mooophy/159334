/**
  *****************************************************************************
  * @file    server.c
  * @author  Yue Wang 12027710
  * @version V0.0.1
  * @date    03-Oct-2013
  * @brief   
   ****************************************************************************
  * @history 0.0.1 function added : unsigned int wy_CRC_check().	
  * 			 0.0.2 function added : unsigned int wy_SeqNum_collect()
  * @bug		 the server can not exit.
  */
//159.334 - Networks 
///////////////   2012 ////////////////////
// SERVER: prototype for assignment 2.
// This code is different than the one used in previous semesters...
//************************************************************************/
//COMPILE WITH: gcc server_Unreliable_UDP_linux2010.c -o server_Unreliable_UDP_linux2010
//with no losses nor damages, RUN WITH: ./server_Unreliable_UDP_linux2010 1235 0 0 
//with losses RUN WITH: ./server_Unreliable_UDP_linux2010 1235 1 0 
//with damages RUN WITH: ./server_Unreliable_UDP_linux2010 1235 0 1 
//with losses and damages RUN WITH: ./server_Unreliable_UDP_linux2010 1235 1 1  
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
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#endif

#include "UDP_supporting_functions_2013.c"

#define BUFFESIZE 80 
//remember, the BUFFESIZE has to be at least big enough to receive the answer from the serv1
#define SEGMENTSIZE 78
#define GENERATOR 0x8005 //0x8005, generator for polynomial division
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.
//********************************************************************
// PROTOTYPE
//********************************************************************
unsigned int CRCpolynomial(char *buffer);
unsigned int wy_CRC_check(char *_buffer);
unsigned int wy_SeqNum_collect(char *_buffer);
//*******************************************************************
//Function to save lines and discarding the header
//*******************************************************************
//You ARE allowed to change this. You will need to alter the NUMBER_OF_WORD_IN_THE_HEADER if you add a CRC
#define NUMBER_OF_WORDS_IN_THE_HEADER 3
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
	else {
		printf("error when trying to write...\n");
		exit(0);
	}
	
}

#if defined __unix__ || defined __APPLE__

#elif defined _WIN32 
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;
#endif

//*******************************************************************
//MAIN
//*******************************************************************
int main(int argc, char *argv[]) {
//********************************************************************
// INITIALIZATION
//********************************************************************
	struct sockaddr_in localaddr,remoteaddr;
#if defined __unix__ || defined __APPLE__
	int s;
#elif defined _WIN32 
	SOCKET s;
#endif
	char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
	int n,bytes,addrlen;
	addrlen = sizeof(remoteaddr);
	memset(&localaddr,0,sizeof(localaddr));//clean up the structure
	memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure
	randominit();
	
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
//********************************************************************
//SOCKET
//********************************************************************
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s <0) {
		printf("socket failed\n");
	}
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
	if (argc != 4) {
		printf("2012 code USAGE: server port  lossesbit(0 or 1) damagebit(0 or 1)\n");
		exit(1);
	}
	localaddr.sin_port = htons((u_short)atoi(argv[1]));
	int remotePort=1234;
	packets_damagedbit=atoi(argv[3]);
	packets_lostbit=atoi(argv[2]);
	if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1){
		printf("2012 code USAGE: server port  lossesbit(0 or 1) damagebit(0 or 1)\n");
		exit(0);
	}
//********************************************************************
//REMOTE HOST IP AND PORT
//********************************************************************
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteaddr.sin_port = htons(remotePort);
	int counter=0;
//********************************************************************
//BIND
//********************************************************************
	if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) {
		printf("Bind failed!\n");
		exit(0);
	}
//********************************************************************
// Open file to save the incoming packets
//********************************************************************
	FILE *fout=fopen("file1_saved.txt","w");
//********************************************************************
//INFINITE LOOP
//********************************************************************
	while (1) {
//********************************************************************
//RECEIVE
//********************************************************************
//printf("Waiting... \n");
#if defined __unix__ || defined __APPLE__
		bytes = recvfrom(s, receive_buffer, SEGMENTSIZE, 0,(struct sockaddr *)(&remoteaddr),(socklen_t*)&addrlen);
#elif defined _WIN32
		bytes = recvfrom(s, receive_buffer, SEGMENTSIZE, 0,(struct sockaddr *)(&remoteaddr),&addrlen);
#endif
//printf("Received %d bytes\n",bytes);
//********************************************************************
//PROCESS REQUEST
//********************************************************************
	//if(bytes=!-1){
		n=0;
		while (n<bytes){
			n++;
			if ((bytes < 0) || (bytes == 0)) break;
			if (receive_buffer[n] == '\n') { /*end on a LF*/
				receive_buffer[n] = '\0';
				break;
			}
			if (receive_buffer[n] == '\r') /*ignore CRs*/
				receive_buffer[n] = '\0';
		}
		if ((bytes < 0) || (bytes == 0)) break;
		printf("\n================================================\n");	
		printf("RECEIVED --> %s \n",receive_buffer);
		//------------
		//printf("YUE : the SN is %d",wy_SeqNum_collect(receive_buffer));
		//printf("the CRC is %d,lenth is %d",CRCpolynomial(receive_buffer),strlen(receive_buffer));
		//printf("len= %d    the last four :%d %d %d %d",strlen(receive_buffer),receive_buffer[33],receive_buffer[34],receive_buffer[35],receive_buffer[36]);
		//------------
	
		if (wy_CRC_check(receive_buffer))  {
			sscanf(receive_buffer, "%d",&counter);
			printf("ffffff");
//********************************************************************
//SEND ACK
//********************************************************************
			sprintf(send_buffer,"ACKNOW %d \r\n",counter);
			send_unreliably(s,send_buffer,remoteaddr);
			save_line_without_header(receive_buffer,fout);
		}
		else {
			if (strncmp(receive_buffer,"CLOSE",5)==0)  {//if client says "CLOSE", the last packet for the file was sent. Close the file
			//Remember that the packet carrying "CLOSE" may be lost or damaged!
				fclose(fout);
#if defined __unix__ || defined __APPLE__
				close(s);
#elif defined _WIN32
				closesocket(s);
#endif	
				printf("Server saved file1_saved.txt \n");//you have to manually check to see if this file is identical to file1.txt
				exit(0);
			}
			else {//it is not PACKET nor CLOSE, therefore there might be a damaged packet
			//in this assignment, CLOSE always arrive (read UDP_supporting_functions_2012.c to see why...)
			//do nothing, ignoring the damaged packet? Or send a negative ACK? It is up to you to decide.
			}
		}
	}
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
  * @brief  return the Sequence Number from the buffer.
  */
unsigned int wy_SeqNum_collect(char *_buffer)
{
	char bf[BUFFESIZE];
	char sep[2] = " "; //separation is space
	char *word;
	int  wcount=0;
	int  SeqNum=1000;
	
	strcpy(bf, _buffer);
	
	for (word = strtok(bf, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount > 2) // jump the first space"_".
		{
			break;
		}
	}
	sscanf(word,"%d",&SeqNum);
	return SeqNum;
}
