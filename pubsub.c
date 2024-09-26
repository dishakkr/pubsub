	#include<stdio.h>
	#include<stdlib.h>
	#include<string.h>
	#include<unistd.h>
	#include<sys/wait.h>
	#include<pthread.h>
	#include<semaphore.h>
	#include "MQTTClient.h"
  	#include <fcntl.h>   
    	#include <termios.h> 
    	#include <errno.h>   
	

#define ADDRESS     "mqtt.mcs.mediatek.com:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "mcs/:DMgqmj0R/:qGwsaFVwJQKIIILl/:temp"
#define TOPIC1      "mcs/:DMgqmj0R/:qGwsaFVwJQKIIILl/:humid"
#define TOPIC2      "mcs/:DMgqmj0R/:qGwsaFVwJQKIIILl/:pressure"
#define PAYLOAD     "---temp data---"
#define PAYLOAD1    "---humidity data---"
#define PAYLOAD2    "---pressure data---"
#define QOS         1
#define TIMEOUT     10000L

sem_t s1, s2;
char *strng[5];
char read_buffer[35];


void* publish(void* data)
{
	while(1){
	sem_wait(&s1);
	MQTTClient client;
        MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
        MQTTClient_message pubmsg = MQTTClient_message_initializer;
        MQTTClient_deliveryToken token;
        int rc;
	MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession = 1;

       		if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
         	{
       			 printf("Failed to connect, return code %d\n", rc);
       			 exit(-1);
         	}

         pubmsg.payload = strng[1];
   	 puts(pubmsg.payload);
         pubmsg.payloadlen = strlen(pubmsg.payload);
	 pubmsg.qos = QOS;
	 pubmsg.retained = 0;
         MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
         printf("Waiting for up to %d seconds......... \nPublication of %s\nTopic: %s \nClientID: %s\n", (int)(TIMEOUT/1000), PAYLOAD, TOPIC, 	CLIENTID);
         rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
         printf("Message with delivery token %d delivered\n", token);

	 pubmsg.payload = strng[2];
   	 puts(pubmsg.payload);
         pubmsg.payloadlen = strlen(pubmsg.payload);
	 pubmsg.qos = QOS;
	 pubmsg.retained = 0;
         MQTTClient_publishMessage(client, TOPIC1, &pubmsg, &token);
         printf("Waiting for up to %d seconds......... \nPublication of %s\nTopic: %s \nClientID: %s\n", (int)(TIMEOUT/1000), PAYLOAD1, TOPIC, 	CLIENTID);
         rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
         printf("Message with delivery token %d delivered\n", token);

	 pubmsg.payload = strng[3];
   	 puts(pubmsg.payload);
         pubmsg.payloadlen = strlen(pubmsg.payload);
	 pubmsg.qos = QOS;
	 pubmsg.retained = 0;
         MQTTClient_publishMessage(client, TOPIC2, &pubmsg, &token);
         printf("Waiting for up to %d seconds......... \nPublication of %s\nTopic: %s \nClientID: %s\n", (int)(TIMEOUT/1000), PAYLOAD2, TOPIC, 	CLIENTID);
         rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
         printf("Message with delivery token %d delivered\n", token);
	 MQTTClient_disconnect(client, 10);
    	 MQTTClient_destroy(&client);
	 sem_post(&s2);
}
}
void* reader(void* data)
{
while(1){
		int fd,i;
		char *str;
		sem_wait(&s2);
		fd = open("/dev/ttyACM0",O_RDWR | O_NOCTTY);
		if(fd == -1)	
			{					
            	   	printf("\n  Error! in Opening ttyACM0  ");
			exit (0);
			} 
	       	else
        	      printf("\n  ttyUSB0 Opened Successfully ");
		struct termios SerialPortSettings;	/* Create the structure                          */

		tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

		/* Setting the Baud rate */
		cfsetispeed(&SerialPortSettings,B9600); /* Set Read  Speed as 9600                       */
		cfsetospeed(&SerialPortSettings,B9600); /* Set Write Speed as 9600                       */

		/* 8N1 Mode */
		SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
		SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
		SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
		SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
		
		SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
		SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
		
		
		SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
		SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

		SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
		
		/* Setting Time outs */
		SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
		SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */


		if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
		    printf("\n  ERROR ! in Setting attributes");
		else
                    printf("\n  BaudRate = 9600 \n  StopBits = 1 \n  Parity   = none\n");
			
	        /*------------------------------- Read data from serial port -----------------------------*/

		tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */
		for(i=0;i<20;i++) read(fd,&read_buffer,35); 

		strng[0]=strtok(read_buffer,":");
		strng[1]=strtok(0,":");
		strng[2]=strtok(0,":");
		strng[3]=strtok(0,":");
		strng[4]=strtok(0,":");
		sem_post(&s1);
}
}

int main(int argc, char *argv[])
{

	pthread_t pubTID,valTID;
	sem_init(&s1,0,0);
	sem_init(&s2,0,1);
	pthread_create(&pubTID,NULL, publish,NULL);
	pthread_create(&valTID,NULL, reader,NULL);
	pthread_join(pubTID,NULL);
	pthread_join(valTID,NULL);
	wait(0);	
return 0;
}
