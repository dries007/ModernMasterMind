/*
 * ramdump.h
 *
 *  Created on: 6-mrt.-2015
 *      Author: Dries007
 */

#ifndef RAMDUMP_H_
#define RAMDUMP_H_

#pragma option -1 //create 80186 code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtos.h>
#include <tcpip.h>
#include <i2c_api.h>
#include <limits.h>
#include <rtxapi.h>
#include <rtos.h>
#include <hwapi.h>
#include <dos.h>
#include <stdarg.h>

#ifndef BYTE_H_
#define BYTE_H_

typedef unsigned char byte;
typedef unsigned short address;

#endif

void ramdump();
void manualram();

#endif /* RAMDUMP_H_ */
