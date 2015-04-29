/*
 * mastermind.h
 *
 *  Created on: 24-apr.-2015
 *      Author: Dries007
 */

#ifndef SRC_MASTERMIND_H_
#define SRC_MASTERMIND_H_

#pragma option -1 //create 80186 code

#include <clib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dos.h>
#include <stdarg.h>
#include <ctype.h>
#include <RTOS.H>
#include <i86.h>

#include "ramdump.h"
#include "httpcli.h"
#include "base64.h"
#include "dns.h"

/* ================================================== DEFINES ==================================================*/

#define DEBUG 0

#define VERSION "0.2"
#define CYEAR "15"

#define TASK_STACKSIZE  2048

#define TCPIP_int 0xAC

#define LCD_LINE_SIZE 20
#define LCD_LINES 4

#define MAX_COLORS 10
#define COLORS 4
#define ROWS 12

#define STATE_NO_GAME 0
#define STATE_GAME_CONFIGURED 1
#define STATE_GAME_STARTED 2
#define STATE_GAME_OVER 3
#define STATE_GAME_WON 4

// amount of (positive) hours offset from GMT
#define TIMEZONE_OFFSET 2

// 170 Leds max (x 3 bytes = 0x1FE)
#define RAM_LEDS_START  0x000
#define RAM_LEDS_END    0x1FE
#define RAM_LEDS_AMOUNT 0x1FF
// 80 bytes of char buffer for LCD
#define RAM_LCD_START   0x200
#define RAM_LCD_END     0x250
#define RAM_LCD_CMD     0x251
// Mask for global LED dimming
#define RAM_LEDS_DIM    0x252
#define RAM_KP_LASTKEY  0x253

#define RAM_VERSION_1   0x300
#define RAM_VERSION_2   0x301

// Interrupt registers
#define RAM_INT_SEND    0x3FF
#define RAM_INT_GET     0x3FE

#define MAX_LEDS        (RAM_LEDS_END - RAM_LEDS_START)
#define MAX_LCD_CHARS   (RAM_LCD_END - RAM_LCD_START)

#define CMD_LEDS_SEND   0x01
#define CMD_LCD_CHAR    0x02
#define CMD_LCD_CMD     0x03
#define CMD_LCD_CL_PR   0x04
#define CMD_LCD_POS     0x05

#define SATUS_KP_PRESS  0x01

#define NORMALIZE_ADDRESS(addr) ((addr & 0x0FF) | 0x100)
#define BANK_FROM_ADDRESS(addr) (addr >> 8)

/* ================================================== STRUCTS ==================================================*/

typedef unsigned char byte;
typedef unsigned short address;

typedef struct
{
    unsigned long ip;
    char name[21];
    byte function;
    byte score;
} User;

typedef struct
{
    byte r;
    byte g;
    byte b;
} RGB;

typedef struct {
    byte state;
    byte vsPlayer;
    User * host;
    byte colors;
    byte code[COLORS];
    byte nrOfGuesses;
    byte guesses[ROWS][COLORS + 2];
} Game;

struct userlist_el
{
   User user;
   struct userlist_el * next;
};

/* ================================================== METHODS ==================================================*/

void endProgram();

User * getUserByIP(long far * ip);
User * getUserByName(char * name);
void addUser(long ip, char name[21]);

Game * getGame();
void resetGame();
void setRndCode(byte colors);
void guessRow(byte id);

void enableDatabus(); // Enables databus
void selectBank(byte bank); // sets PIO pins 2 and 3 to out & defaults them to the selected bank [0 .. 3]
byte readDatabus(address addr); // read byte from databus
void writeDatabus(address addr, byte value); // write byte to databus
void initTime();

void setLCDLine(byte line, const char *format, ...); // Clear line and print to LCD

void installCGIMethods();
void removeCGIMethods();

void printAllUsers();

/* ================================================== GLOBALS ==================================================*/

const RGB BLACK = {0, 0, 0};
const RGB WHITE = {255, 255, 255};

const RGB RED = {255, 0, 0};
const RGB GREEN = {0, 255, 0};
const RGB BLUE = {0, 0, 255};

const RGB PINK = {255, 0, 255};
const RGB AQUA = {0, 255, 255};
const RGB YELLOW = {255, 255, 0};

const RGB PURPLE = {130, 0, 255};
const RGB ORANGE = {255, 130, 0};

//                                      RED         GREEN       BLUE        PINK            AQUA            YELLOW      PURPLE          ORANGE          WHITE           BLACK
const RGB ALL_COLORS[MAX_COLORS] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 0}, {130, 0, 255}, {255, 130, 0}, {255, 255, 255}, {0, 0, 0}};
const char * ALL_COLOR_CLASSES[MAX_COLORS] = {"red", "green", "blue", "pink", "aqua", "yellow", "purple", "orange", "white", "black"};

union REGS inregs;
union REGS outregs;
struct SREGS segregs;

Game game;

struct userlist_el * listHead = NULL;
struct userlist_el * listTail = NULL;

#endif /* SRC_MASTERMIND_H_ */
