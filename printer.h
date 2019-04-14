/*
 * printer.h
 *
 *  Created on: Apr. 12, 2019
 *      Author: mathieu
 */

#ifndef PRINTER_H_
#define PRINTER_H_

#include <stdint.h>

int initPrinter(const char*);
void writeBytes(char*, int);
int printBitmap(int, int, uint8_t*);
void feed(uint8_t);

#define QR_CODE_WIDTH 		135
#define QR_CODE_HEIGHT		135

struct printerMessage{
	uint32_t numberOfPrint;	// number of print
	uint8_t bitmap[QR_CODE_WIDTH*QR_CODE_HEIGHT]; // bitmap
};

typedef struct printerMessage printerMessage_t;

#endif /* PRINTER_H_ */
