/*
 * RPI_Printer_main.c
 *
 *  Created on: Apr. 9, 2019
 *      Author: mathieu
 */

#include "clientPi.h"
#include "communication.h"
#include "printer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define __PRINTER_CONNECTED

// socket
static int socketFdCmd = -1;



// thread declaration
pthread_t commandThread;
void *commandThreadFunction(void*);

int main(int argc, const char* argv[])
{
	// init printer
#ifdef __PRINTER_CONNECTED
	int printerIniterror = initPrinter("/dev/ttyS0");

	if(printerIniterror < 0)
	{
		perror("Error while initializng prinetr");
		exit(1);
	}
#endif

	const char* serverName = "math-main-pi.local";

	socketFdCmd = initClient(serverName, COMMAND_PORT);
	while(socketFdCmd < 0)
	{
		sleep(1);
		socketFdCmd = initClient(serverName, COMMAND_PORT);
	}

	printf("connexion établis \n");

	// Send message to discover client
	char *data = "RPI-Printer";

	sendMessage(socketFdCmd, data, strlen(data), ID_MSG, RPI_PRINTER_ID);

	// create and start threads
	pthread_attr_t attribute;
	pthread_attr_init(&attribute);

	// create and start thread
	pthread_create(&commandThread, &attribute, commandThreadFunction,&socketFdCmd);

	// wait for thread
	pthread_join(commandThread, NULL);

	printf("l'application est terminée ! \n");

	return 0;
}

/*
 * Thread function for command from interface
 */
void *commandThreadFunction(void* param)
{
	int socketFd = *((int*)(param));

	// initialize command
	RPIMessageHeader_t *header = (RPIMessageHeader_t*)malloc(sizeof(RPIMessageHeader_t));
	char *message = (char*)malloc(QR_CODE_WIDTH*QR_CODE_HEIGHT + 4);

	while(1)
	{
		if(receiveMessage(socketFd, header, message) < 0)
			break;

		switch(header->PayloadType)
		{
			case PRINT_CMD :
				printf("Number of print : %d \n", ((printerMessage_t*)message)->numberOfPrint);
#ifdef __PRINTER_CONNECTED
				int i = 0;
				int printNumber = ((printerMessage_t*)message)->numberOfPrint;
				for(i = 0;i < printNumber; i++)
				{
					printBitmap(QR_CODE_WIDTH, QR_CODE_HEIGHT, &(((printerMessage_t*)message)->bitmap[0]));
					feed(5);
				}
#endif
				break;

			default:
				break;

		}

	}

	pthread_exit(NULL);

}

