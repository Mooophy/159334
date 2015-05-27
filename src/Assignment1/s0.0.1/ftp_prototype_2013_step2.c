/**
  *****************************************************************************
  * @file    FTP_server.c
  * @author  Yue Wang 12027710
  * @version V0.0.1
  * @date    13-Oct-2013
  * @brief   159.334 Assignment 1
   ****************************************************************************
  */
#include <windows.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <winsock.h>
#include <winsock2.h>

#define BUFFERSIZE 800
#define WSVERS MAKEWORD(2,0)

WSADATA wsadata;

void wy_fileName_collector(char	*_buffer, char	*_nameBuffer);

//MAIN
int main(int argc, char *argv[]) 
{
	//INITIALIZATION
   struct sockaddr_in localaddr,remoteaddr;
   struct sockaddr_in remoteaddr_act;
	
   SOCKET s,ns;
   SOCKET s_data_act;

   char send_buffer[BUFFERSIZE],receive_buffer[BUFFERSIZE];

   int remotePort;
   int localPort;//no need for local IP...
   int n,bytes,addrlen;
   memset(&localaddr,0,sizeof(localaddr));//clean up the structure
   memset(&localaddr,0,sizeof(remoteaddr));//clean up the structure

	//WSASTARTUP
   if (WSAStartup(WSVERS, &wsadata) != 0) 
	{
	   WSACleanup();
	   printf("WSAStartup failed\n");
	   exit(1);
   }

	//SOCKET
   s = socket(PF_INET, SOCK_STREAM, 0);
   if (s <0) 
	{
      printf("socket failed\n");
   }
   localaddr.sin_family = AF_INET;
   if (argc == 2) localaddr.sin_port = htons((u_short)atoi(argv[1]));
   else localaddr.sin_port = htons(1234);//default listening port 
   localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local	
	
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
      if (ns <0 ) break;
      printf("accepted a connection from client IP %s port %d \n",inet_ntoa(remoteaddr.sin_addr),ntohs(localaddr.sin_port));
		
      //Respond with welcome message
      sprintf(send_buffer,"220 Welcome \r\n");
      bytes = send(ns, send_buffer, strlen(send_buffer), 0);
      
		while (1) 
		{
         n = 0;
         while (1) 
			{
				//RECEIVE
            bytes = recv(ns, &receive_buffer[n], 1, 0);//receive byte by byte...

				//PROCESS REQUEST
            if ( bytes <= 0 ) break;
            if (receive_buffer[n] == '\n') 
				{ /*end on a LF*/
               receive_buffer[n] = '\0';
               break;
            }
            if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
         }
         if ( bytes <= 0 ) break;

			printf("-->DEBUG: the message from client reads: '%s' \r\n", receive_buffer);
			
			if (strncmp(receive_buffer,"USER",4)==0)  
			{
				printf("Logging in \n");
				sprintf(send_buffer,"331 Password required \r\n");
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            if (bytes < 0) break;
			}
			
			if (strncmp(receive_buffer,"PASS",4)==0)  
			{
				printf("Typing password (anything will do... \n");
				sprintf(send_buffer,"230 Public login sucessful \r\n");
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            if (bytes < 0) break;
			}
			
			if (strncmp(receive_buffer,"SYST",4)==0)  
			{
				printf("Information about the system \n");
				sprintf(send_buffer,"230 I don't know which OS this is: A very naive FTP system... \r\n");
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            if (bytes < 0) break;
			}

			//PORT
			if(strncmp(receive_buffer,"PORT",4)==0) 
			{
				s_data_act = socket(AF_INET, SOCK_STREAM, 0);
               	 //local variables
				unsigned char act_port[2];
				int act_ip[4], port_dec;
				char ip_decimal[40];
				sscanf(receive_buffer, "PORT %d,%d,%d,%d,%d,%d",&act_ip[0],&act_ip[1],&act_ip[2],&act_ip[3],(int*)&act_port[0],(int*)&act_port[1]);
				remoteaddr_act.sin_family=AF_INET;//local_data_addr_act
				sprintf(ip_decimal, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2],act_ip[3]);
				printf("IP is %s\n",ip_decimal);
				remoteaddr_act.sin_addr.s_addr=inet_addr(ip_decimal);
				port_dec=act_port[0]*256+act_port[1];
				printf("port %d\n",port_dec);
				remoteaddr_act.sin_port=htons(port_dec);
				
				if (connect(s_data_act, (struct sockaddr *)&remoteaddr_act, (int) sizeof(struct sockaddr)) != 0)
				{
					printf("trying connection in %s %d\n",inet_ntoa(remoteaddr_act.sin_addr),ntohs(remoteaddr_act.sin_port));
					sprintf(send_buffer, "425 Something is wrong, can't start the active connection... \r\n");
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);

					closesocket(s_data_act);
				}
				else 
				{
					sprintf(send_buffer, "200 Ok\r\n");
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					printf("Data connection to client created (active connection) \n");
				}
			}
			
			/**
			  * @brief  RETR
			  */
			if((strncmp(receive_buffer,"RETR",4)==0))
			{
				char fileName[40];
				wy_fileName_collector(receive_buffer,	fileName);
				printf("@alan file name=%s\n",	fileName);
				break;				
			}
			
			

			//LIST or NLST                                       
			if ( (strncmp(receive_buffer,"LIST",4)==0) || (strncmp(receive_buffer,"NLST",4)==0))   
			{ 
				system("dir > tmp.txt");
		
				FILE *fin=fopen("tmp.txt","r");//open tmp.txt file
				sprintf(send_buffer,"150 Transfering... \r\n");
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				char temp_buffer[80];
				while (!feof(fin))
				{
					fgets(temp_buffer,78,fin);
					sprintf(send_buffer,"%s",temp_buffer);
					send(s_data_act, send_buffer, strlen(send_buffer), 0);
				}
				fclose(fin);
				sprintf(send_buffer,"226 File transfer completed... \r\n");
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			
				closesocket(s_data_act);
				}
			if (strncmp(receive_buffer,"QUIT",4)==0)  
			{
				printf("Quit \n");
				sprintf(send_buffer,"221 Connection closed by the FTP client zzzzzzzzzzzzz\r\n");
				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				if (bytes < 0) break;
				closesocket(ns);
			}
		}	
		//CLOSE SOCKET
		closesocket(ns);
		printf("disconnected from %s\n",inet_ntoa(remoteaddr.sin_addr));
	}
   closesocket(s);//it actually never gets to this point....use CTRL_C
}

void wy_fileName_collector(char	*_buffer, char	*_nameBuffer)
{
	char bf[BUFFERSIZE];	
	char sep[2] = " "; //separation is space
	char *word;
	int  wcount=0;
		
	strcpy(bf, _buffer);
	for (word = strtok(bf, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount == 2) // jump the first space"_".
		{
			strcpy(_nameBuffer,	word);
		}	
	}
}
