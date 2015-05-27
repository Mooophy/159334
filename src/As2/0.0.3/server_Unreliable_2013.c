/**
  *****************************************************************************
  * @file    server.c
  * @author  Yue Wang 12027710
  * @version V0.0.3
  * @date    03-Oct-2013
  * @brief   
   ****************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include "UDP_supporting_functions_2013.c"

/* MACRO ---------------------------------------------------------------------*/
#define BUFFESIZE 80 
#define SEGMENTSIZE 78
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.
#define WSVERS MAKEWORD(2,0)
#define NUMBER_OF_WORDS_IN_THE_HEADER 2 // used in save_line_without_header.

/* VARIABLES-------------------------------------------------------------------*/
WSADATA wsadata;
/* Prototype-------------------------------------------------------------------*/
void save_line_without_header(char * receive_buffer,FILE *fout);
int  wy_SN_check(int _SN_expected, char *_buffer);

/* MAIN------------------------------------------------------------------------*/
int main(int argc, char *argv[]) 
{	
	//Initialization
	struct sockaddr_in localaddr,remoteaddr;
 
	SOCKET s;
	char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
	int n,bytes,addrlen;
	
	int SN_expected = 0; //added.
	int SN_checked = -1; //added.
	
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
	int counter=0;

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
		
		
		
	  /**
		 * @brief  Coding from here
		 */
		printf("Alan: when the expected SN is 3...%d...",wy_SN_check(3, receive_buffer));
		
		
		if (strncmp(receive_buffer,"PACKET",6)==0)   //check
		{
			sscanf(receive_buffer, "PACKET %d",&counter);
			
			//SEND ACK
			sprintf(send_buffer,"ACKNOW %d \r\n",counter);
			send_unreliably(s,send_buffer,remoteaddr);
			
			//save the received line without header
			save_line_without_header(receive_buffer,fout); 
		}
		else 
		{
			if (strncmp(receive_buffer,"CLOSE",5)==0)  
			{//if client says "CLOSE", the last packet for the file was sent. Close the file
			//Remember that the packet carrying "CLOSE" may be lost or damaged!
				fclose(fout);
				closesocket(s);
				printf("Server saved file1_saved.txt \n");//you have to manually check to see if this file is identical to file1.txt
				exit(0);
			}
			else 
			{//it is not PACKET nor CLOSE, therefore there might be a damaged packet
			//in this assignment, CLOSE always arrive (read UDP_supporting_functions_2012.c to see why...)
			//do nothing, ignoring the damaged packet? Or send a negative ACK? It is up to you to decide.
			}
			
	 /**
	   * @brief  coding end.
	   */
		}
	}

	closesocket(s);
	exit(0);
}


/* Function definitions  ------------------------------------------------------*/
/**
  * @brief  return 1 when the received is equae to the expected. 0 for else. 
  */
int wy_SN_check(int _SN_expected, char *_buffer)
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
		if(wcount == 3) // jump the first 2 space"_".
		{
			sscanf(word,"%d",&SN_extracted);
		}	
	}
	return _SN_expected == SN_extracted;	
}
/**
  * @brief  save lines and discarding the header
  */
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
