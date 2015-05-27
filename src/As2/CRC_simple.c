//159.334 Computer Network
//this is a simple 16 bits CRC function
//it does not uses reverse data or reverse result
//the Initial remainder is 0x0000, the generator is 0x8005
//Usage:
//the input should be just a normal string, finishing in '\0' (careful with including or not '\n')
//From the sender's side, you can just use the function, and concatenate the result of the CRC
//The CRC needs to be sent with the message, as a text (in decimal notation, or any other notation you want)
//The receiver needs to be able to split the CRC and the message, recompute the CRC and compare.
#include <stdio.h>
#include <string.h>

#define GENERATOR 0x8005 //0x8005, generator for polynomial division
//#define GENERATOR 0x1021 

//for the assignments you can copy and past CRCpolynomial() directly in your C code.
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

///////////// This is an example only, to show how to use the CRCpolynomial(char*)   ///////////////
//////////// if you want to include this file in your client and server, comment the main() function before /////////
int main(){
	char buffer[1025];
	strcpy((char *)buffer, "PACKET 0 line 1 aaaaaaaaaaaaaaaaaaaa");
	//strcpy((char *)buffer, "A");1
	unsigned int CRCresult;
	CRCresult=CRCpolynomial(buffer);
	printf("The CRC hex value for: '%s' is lenght is %d ",buffer,strlen(buffer));
	printf(" 0x%X\n\n",CRCresult);
	printf(" %d\n\n",CRCresult);
	return 0;
}
