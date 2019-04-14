/*
 * printer.c
 *
 *  Created on: Apr. 12, 2019
 *      Author: mathieu
 */

#include "printer.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>

// defines
#define ASCII_TAB '\t' // Horizontal tab
#define ASCII_LF  '\n' // Line feed
#define ASCII_FF  '\f' // Form feed
#define ASCII_CR  '\r' // Carriage return
#define ASCII_DC2  18  // Device control 2
#define ASCII_ESC  27  // Escape
#define ASCII_FS   28  // Field separator
#define ASCII_GS   29 // Group separator


#define BAUDRATE 				19200
#define BYTE_TIME 				(((11L * 1000000L) + (BAUDRATE / 2)) / BAUDRATE)
#define DEFAULT_HEAT_TIME 		120

static int fdPrinter = -1;

static uint8_t prevByte = 0;
static uint8_t column = 0;
static uint8_t maxColumn = 0;
static uint8_t charHeight = 0;
static uint8_t lineSpacing = 0;
static uint8_t barcodeHeight = 50;
int dotPrintTime   = 0; // See comments near top of file for
int dotFeedTime    =  0; // an explanation of these values.
int maxChunkHeight = 0;


int initPrinter(const char *pPath)
{
	// open device
	fdPrinter = open(pPath, O_RDWR | O_NOCTTY);

	if(fdPrinter < 0)
	{
		perror("Error while opening device (printer)");
		return fdPrinter;
	}

	// configure device
	struct termios *options = (struct termios*) malloc(sizeof(struct termios));
	tcgetattr(fdPrinter, options);

	if(!options)
	{
		perror("Error while getting attributes");
		return -2;
	}

	// configuring UART
	options->c_cflag = B19200 | CS8 | CLOCAL | CREAD; // baud = 19200, 8 bits
	options->c_iflag = IGNPAR; // no parity bit
	options->c_oflag = 0;
	options->c_lflag = 0;

	// flush and set attributes
	tcflush(fdPrinter, TCIFLUSH);
	tcsetattr(fdPrinter, TCSANOW, options);

	// no error
	free(options);

	// init printer //

	// wake up printer //
	char wakeUp[] = {255};
	write(fdPrinter,&wakeUp, 1);

	usleep(50000); // wait 50 ms

	char byteToWrite[] = {ASCII_ESC, '8', 0, 0};
	writeBytes(&byteToWrite[0], 4);


	// reset //
	char initCmd[] = {ASCII_ESC, '@'};
	writeBytes(&initCmd[0], 2);

	prevByte      = '\n';       // Treat as if prior line is blank
	column        =    0;
	maxColumn     =   32;
	charHeight    =   24;
	lineSpacing   =    6;
	barcodeHeight = 50;

	// Configure tab stops on recent printers
	char cmd1[] = {ASCII_ESC, 'D'};
	writeBytes(&cmd1[0], 2);

	char cmd2[] = {4,8,12,16};
	writeBytes(&cmd2[0], 4);

	char cmd3[] = {20, 24, 28, 0};
	writeBytes(&cmd3[0], 4);

	// ESC 7 parameters
	char cmd4[] = {ASCII_ESC, '7'}; // printf setting
	writeBytes(&cmd4[0], 2);

	char cmd5[] = {11, DEFAULT_HEAT_TIME, 40};
	writeBytes(&cmd5[0], 2);

	char cmd6[] = {ASCII_DC2, '#', (2 << 5) | 10}; // 2 = print density, 10 = printBreakTime
	writeBytes(&cmd6[0], 2);

	dotPrintTime   = 30000; // See comments near top of file for
	dotFeedTime    =  2100; // an explanation of these values.
	maxChunkHeight = 255;

	return 0;

}

int printBitmap(int w, int h, uint8_t*bitmap)
{
	int rowBytes, rowBytesClipped, rowStart, chunkHeight, chunkHeightLimit,
	      x, y, i;

	char *cmd = (char*)malloc(4);
	memset(cmd, 0, 4);

	rowBytes        = (w + 7) / 8; // Round up to next byte boundary
	rowBytesClipped = (rowBytes >= 48) ? 48 : rowBytes; // 384 pixels max width

	chunkHeightLimit = 256 / rowBytesClipped;
	if(chunkHeightLimit > maxChunkHeight) chunkHeightLimit = maxChunkHeight;
	else if(chunkHeightLimit < 1)         chunkHeightLimit = 1;

	for(i=rowStart=0; rowStart < h; rowStart += chunkHeightLimit) {

		// Issue up to chunkHeightLimit rows at a time:
		chunkHeight = h - rowStart;
		if(chunkHeight > chunkHeightLimit) chunkHeight = chunkHeightLimit;

		// write command
		cmd[0] = ASCII_DC2;
		cmd[1] = '*';
		cmd[2] = chunkHeight;
		cmd[3] = rowBytesClipped;
		writeBytes(cmd, 4);

		for(y=0; y < chunkHeight; y++) {
		  for(x=0; x < rowBytesClipped; x++, i++) {
			usleep(10);
			// write to printer
			writeBytes((char*)(bitmap + i), 1);
		  }
		  i += rowBytes - rowBytesClipped;
		}
	}
	prevByte = '\n';

	return 0;
}

void feed(uint8_t numberOfLine)
{
	char cmd[] = {ASCII_ESC, 'd', numberOfLine};
	writeBytes(&cmd[0], 3);
	usleep(dotFeedTime * charHeight);
	prevByte = '\n';
	column = 0;
}

void writeBytes(char* byteToWrite, int numberOfBytes)
{
	// write bytes
	write(fdPrinter, byteToWrite, numberOfBytes);

	// sleep
	usleep(numberOfBytes*BYTE_TIME);
}


