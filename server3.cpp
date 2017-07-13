#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "wiringPi.h"
#include "wiringPiSPI.h"

int chan=0;

void read(uint8_t *arry,uint16_t *gx,uint16_t *gy,uint16_t *gz,uint16_t *temp,uint16_t g_werte[]);
void write1(uint8_t *arry, uint8_t command,uint8_t val);

 int GetZahlLaenge(int zahl)
        {
            int laenge = 0;
            int teiler = 1;
            while (zahl / teiler >= 1)
            { laenge++; teiler = teiler * 10; }
 
            return laenge;
        }

void * get_in_addr(struct sockaddr * sa)
{
	if(sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr); 
	}
	
	return &(((struct sockaddr_in6 *)sa)->sin6_addr); 
}

int main(int argc, char * argv[])
{
	// Variables for writing a server. 
	/*
	1. Getting the address data structure.
	2. Openning a new socket.
	3. Bind to the socket.
	4. Listen to the socket. 
	5. Accept Connection.
	6. Receive Data.
	7. Close Connection. 
	*/
	int status;
	struct addrinfo hints, * res;
	int listner; 
	int zahl=-32501;
	char buffer[256];
	uint16_t data[4];
	uint8_t i,g=0,l=255;
	int32_t speed=10000000;
	uint8_t a;
	uint16_t g_werte[7],gx=0,gy=0,gz=0;
	uint16_t ax=0,ay=0,az=0;
	uint16_t xx=0,yy=0,zz=0;
	uint8_t command=0, write=0;
	uint8_t  buff[6],arry[2];
	uint16_t temp=0;
	
	
	for(i=0;i<6;i++) buff[i]=0x00;

	if( wiringPiSPISetup (chan, speed)==-1)
	{
	printf("Could not initialise SPI\n");
	return 0;
	}

	if( wiringPiSetup()==-1)
	{
	printf("Could not initialise \n");
	return 0;
	}

pinMode(6,INPUT);


	write1(buff,0x6b,0x00);
	delay(100);
	write1(buff,0x6b,0x01);
	delay(200);
	write1(buff,0x1a,0x00);
	delay(20);
	write1(buff,0x19,0x04);
	delay(20);
	write1(buff,0x23,0xf8);
	delay(20);
	write1(buff,0x1B,0x18);//Gyro Config
	delay(20);
	write1(buff,0x1C,0x18); //ACC Config
	delay(20);
	write1(buff,0x1D,0x2); //ACC2 Config
	delay(20);
	write1(buff,0x37,0x22); //Int_Pin_Cfg 
	delay(20);
	write1(buff,0x38,0x01); //Int_Enable 
	delay(100);
	
	// Before using hint you have to make sure that the data structure is empty 
	memset(& hints, 0, sizeof hints);
	// Set the attribute for hint
	hints.ai_family = AF_UNSPEC; // We don't care V4 AF_INET or 6 AF_INET6
	hints.ai_socktype = SOCK_STREAM; // TCP Socket SOCK_DGRAM 
	hints.ai_flags = AI_PASSIVE; 
	
	// Fill the res data structure and make sure that the results make sense. 
	status = getaddrinfo(NULL, "8888" , &hints, &res);
	if(status != 0)
	{
		fprintf(stderr,"getaddrinfo error: %s\n",gai_strerror(status));
	}
	
	// Create Socket and check if error occured afterwards
	listner = socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(listner < 0 )
	{
		fprintf(stderr,"socket error: %s\n",gai_strerror(status));
	}
	
	// Bind the socket to the address of my local machine and port number 
	status = bind(listner, res->ai_addr, res->ai_addrlen); 
	if(status < 0)
	{
		fprintf(stderr,"bind: %s\n",gai_strerror(status));
	}

	status = listen(listner, 10); 
	if(status < 0)
	{
		fprintf(stderr,"listen: %s\n",gai_strerror(status));
	}
	
	// Free the res linked list after we are done with it	
	freeaddrinfo(res);
	
	
	// We should wait now for a connection to accept
	int new_conn_fd;
	struct sockaddr_storage client_addr;
	socklen_t addr_size;
	char s[INET6_ADDRSTRLEN]; // an empty string 
		
	// Calculate the size of the data structure	
	addr_size = sizeof client_addr;
	
	printf("I am now accepting connections ...\n");
	
	while(1){
		// Accept a new connection and return back the socket desciptor 
		new_conn_fd = accept(listner, (struct sockaddr *) & client_addr, &addr_size);	
		if(new_conn_fd < 0)
		{
			fprintf(stderr,"accept: %s\n",gai_strerror(new_conn_fd));
			continue;
		}
	    char s99[12] ;
		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *) &client_addr),s ,sizeof s); 
		printf("I am now connected to %s \n",s);
		sprintf(s99, "%d", zahl);
		int laenge_der_zahl=GetZahlLaenge(zahl);
		while(1){
		read(buff,&gx,&gy,&gz,&temp,g_werte);
			
			for(uint8_t kk=0;kk<7;kk++){
			sprintf(s99, "%d", g_werte[kk]);
			laenge_der_zahl=GetZahlLaenge(g_werte[kk]);
			status = send(new_conn_fd,s99, laenge_der_zahl,2);
			status = send(new_conn_fd,"\r\n", 2,2);
			}
			printf("\nGwertGX=%u GwertGY=%u GwertGZ=%u Temp=%u\n",g_werte[0],g_werte[1],g_werte[2],g_werte[3]);
			printf("\nGwertAX=%u GwertAY=%u GwertAZ=%u \n",g_werte[4],g_werte[5],g_werte[6]);
			
		}
		if(status == -1)
		{printf("nerror -1");
			close(new_conn_fd);
			_exit(4);
		}
		
	}
	// Close the socket before we finish 
	close(new_conn_fd);	
	
	return 0;
}

void read(uint8_t *arry,uint16_t *gx,uint16_t *gy,uint16_t *gz,uint16_t *temp,uint16_t g_werte[])
{
	
  //////////////////////////////////////////////////////////////////////////////////////////////////////
	arry[0]=0x43|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t gxMSB = arry[1];

	arry[0]=0x44|0x80;
	wiringPiSPIDataRW (chan,arry,2);
 
  
	uint8_t gxLSB = arry[1];
	*gx = (uint16_t)((gxMSB << 8) | gxLSB);
	g_werte[0]=(uint16_t)((gxMSB << 8) | gxLSB);
  //////////////////////////////////////////////////////////////////////////////////////////////////////
	arry[0]=0x45|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t gyMSB = arry[1];

	arry[0]=0x46|0x80;
	wiringPiSPIDataRW (chan,arry,2);
 
  
	uint8_t gyLSB = arry[1];
	*gy = (uint16_t)((gyMSB << 8) | gyLSB);
	g_werte[1]= (uint16_t)((gyMSB << 8) | gyLSB);
  //////////////////////////////////////////////////////////////////////////////////////////////////////
	arry[0]=0x47|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	int8_t gzMSB = arry[1];

	arry[0]=0x48|0x80;
	wiringPiSPIDataRW (chan,arry,2);
 
  
  uint8_t gzLSB = arry[1];
  *gz = (uint16_t)((gzMSB << 8) | gzLSB);
 g_werte[2]=(uint16_t)((gzMSB << 8) | gzLSB);
	arry[0]=0x41|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t tempMSB = arry[1];
	
	arry[0]=0x42|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t tempLSB = arry[1];
	
	*temp = (uint16_t)((tempMSB << 8) | tempLSB);
g_werte[3]=(uint16_t)((tempMSB << 8) | tempLSB);
 //////////////////////////////////////////////////////////////////////////////////////////////////////
	arry[0]=0x3b|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t axMSB = arry[1];

	arry[0]=0x3c|0x80;
	wiringPiSPIDataRW (chan,arry,2);
 
  
	uint8_t axLSB = arry[1];

	g_werte[4]=(uint16_t)((axMSB << 8) | axLSB);
  //////////////////////////////////////////////////////////////////////////////////////////////////////
	arry[0]=0x3d|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t ayMSB = arry[1];

	arry[0]=0x3e|0x80;
	wiringPiSPIDataRW (chan,arry,2);
 
  
	uint8_t ayLSB = arry[1];
	
	g_werte[5]=(uint16_t)((ayMSB << 8) | ayLSB);
  //////////////////////////////////////////////////////////////////////////////////////////////////////	
  arry[0]=0x3f|0x80;
	wiringPiSPIDataRW (chan,arry,2);
	uint8_t azMSB = arry[1];

	arry[0]=0x40|0x80;
	wiringPiSPIDataRW (chan,arry,2);
 
  
	uint8_t azLSB = arry[1];
	
	g_werte[6]=(uint16_t)((azMSB << 8) | azLSB);
  //////////////////////////////////////////////////////////////////////////////////////////////////////	
}

void write1(uint8_t *arry, uint8_t command,uint8_t val)
{
int8_t i,x;	
	arry[0]=command;
	arry[1]=val;
	wiringPiSPIDataRW (chan,arry,2);
	for(x=1;x<2;x++)
	printf ("%x  \n", arry[x]);

}