//159.334 - Networks
// 2013 code 
// This code is different than the one used in previous semesters...
//*******************************************************************************************************************************//
//you are NOT allowed to change these functions
//*******************************************************************************************************************************//
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#if defined __unix__ || defined __APPLE__
//nothing to do in unix
#elif defined _WIN32 
	typedef struct { unsigned long l,h; } ti;
	typedef struct { unsigned long sz, ml, tp, ap, tpg, apg, tv, av; } ms;
		
	#ifdef __cplusplus
	extern "C" {
	#endif
		void * _stdcall GetCurrentProcess(void);
		unsigned long _stdcall GetVersion(void);
	//void * _stdcall GetProcessTimes(void *, ti *,ti *, ti *, ti *);
	#ifdef __cplusplus 
	}
	#endif
	int cputime(void) { // return cpu time used by current process
		ti ct, et, kt, ut;
		if (GetVersion() < 0x80000000) {  // are we running on NT/2000/XP
			GetProcessTimes(GetCurrentProcess(), (_FILETIME*)&ct, (_FILETIME*)&et, (_FILETIME*)&kt, (_FILETIME*)&ut);
			return (ut.l + kt.l) / 10000; // include time in kernel
		}
		else return clock(); // for Windows 95/98/Me
	}
#endif


//********************************************
// fate=0 (ok)  or 
//     =1 (damaged) or 
//     =2 (lost)
// the parameters for damage or losses are part of the arguments on both server and client
//********************************************
int packets_damagedbit=0;
int packets_lostbit=0;
//deprecated constants
//#define PACKETS_CAN_BE_DAMAGED 1 
//#define PACKETS_CAN_BE_LOST 1
#if defined __unix__ || defined __APPLE__
struct timespec time1;
#endif
void randominit(void){
#ifdef __unix__
	srand((unsigned)time(NULL));
#elif defined _WIN32 
	srand(cputime());	
#endif
}


//changes introduced to get, on average, 20% losses and 20% damage while 60% of the packets should go through (resp=0)
int packets_fate(void){
	if (packets_damagedbit==0 && packets_lostbit==0) return 0;
	if (packets_damagedbit==1 && packets_lostbit==0) {
int resp=(int) (10.0 * (rand() / (RAND_MAX + 1.0)));
//		int resp=rand()%10;
		if (resp==1 || resp==3 || resp==9 ) return 1;
		else return 0;
	}
	if (packets_damagedbit==0 && packets_lostbit==1) {
int resp=(int) (10.0 * (rand() / (RAND_MAX + 1.0)));
//		int resp=rand()%10;
		if (resp==2 || resp==4 || resp==6) return 2;
		else return 0;
	}
	if (packets_damagedbit==1 && packets_lostbit==1) {
int resp=(int) (10.0 * (rand() / (RAND_MAX + 1.0)));
//		int resp=rand()%10;
		if (resp==2 || resp==4 || resp==6) return 2;
		if (resp==1 || resp==3 || resp==9) return 1;
		else return 0;
	}
	return 0;
}

int damage_bit(void){
	return (int) (7+(40.0 * (rand() / (RAND_MAX + 1.0))));//7 added guarantees that PACKET and CLOSE will not be damaged...
}

int random_char(void){
	return (int) (255.0 * (rand() / (RAND_MAX + 1.0)));
}

int send_unreliably(int s,char * send_buffer, struct sockaddr_in remoteaddress) {
	int fate=packets_fate();
	if (strncmp(send_buffer,"CLOSE",5)==0) fate=0;//this is to guarantee no loss on CLOSE (otherwise the server gets stuck).
	if (fate==0){//fate=0 (ok)  or 1 (damaged) or 2 (lost)
		int bytes = sendto(s, send_buffer, strlen(send_buffer),0,(struct sockaddr *)(&remoteaddress),sizeof(remoteaddress) );
		printf("<-- SEND %s \n",send_buffer);
		if (bytes < 0) {
			printf("send failed\n");
			exit(1);
		}
	}
	else {
		if (fate== 1){
			char copy_of_buffer[100];
			strcpy(copy_of_buffer,send_buffer);
			printf("TRIED %s ",copy_of_buffer);
			copy_of_buffer[damage_bit()]=random_char();
			copy_of_buffer[damage_bit()]=random_char();
			int bytes = sendto(s, copy_of_buffer, strlen(copy_of_buffer),0,(struct sockaddr *)(&remoteaddress),sizeof(remoteaddress) );
			printf("<-- DAMAGED %s \n",copy_of_buffer);
			if (bytes < 0) {
				printf("send failed\n");
				exit(1);
			}
		}
		if(fate==2){
			printf("X-- LOST %s \n",send_buffer);
//do nothing, packet is lost
		}
	}
	return 0;
}
//*******************************************************************************************************************************//
//*******************************************************************************************************************************//
//*******************************************************************************************************************************//

void recv_unreliably_blocking(int s,char * receive_buffer, struct sockaddr_in remoteaddr){
	int bytes=0;
	int addrlen = sizeof(remoteaddr);
#if defined __unix__ || defined __APPLE__
	bytes = recvfrom(s, receive_buffer, 78, 0,(struct sockaddr *)(&remoteaddr),(socklen_t*)&addrlen);//blocking
#elif defined _WIN32 
	bytes = recvfrom(s, receive_buffer, 78, 0,(struct sockaddr *)(&remoteaddr),&addrlen);//blocking
#endif	
//********************************************************************
//PROCESS REQUEST
//********************************************************************
	int n=0;
	if (bytes !=-1){
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
	}
}

void recv_nonblocking(int s,char * receive_buffer, struct sockaddr_in remoteaddr){
	int bytes=0;
	int addrlen = sizeof(remoteaddr);
#if defined __unix__ || defined __APPLE__
	bytes = recvfrom(s, receive_buffer, 78, MSG_DONTWAIT,(struct sockaddr *)(&remoteaddr),(socklen_t*)&addrlen);//non-blocking
#elif defined _WIN32 
	bytes = recvfrom(s, receive_buffer, 78, 0,(struct sockaddr *)(&remoteaddr),&addrlen);//non-blocking
#endif
//********************************************************************
//PROCESS REQUEST
//********************************************************************
	int n=0;
	if (bytes !=-1){
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
	}
}
