/*
 The MIT License (MIT)

 Copyright (c) 2015 Dries007

 Made for/in relation to an education at Thomas More Mechelen-Antwerpen vzw
 Campus De Nayer - Professionele bachelor elektronica-ict

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
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
