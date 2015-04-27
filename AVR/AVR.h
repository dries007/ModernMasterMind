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
#ifndef AVR_H_
#define AVR_H_

#define DEBUG 0

/* =============== PORT CONFIG ============== */
// Mask to eliminate INT4
#define	LCD_PORT		PORTE
#define	LCD_DDR			DDRE
#define	LCD_MASK		0xEF
// KP = Keypad
#define	KP_PORT			PORTD
#define	KP_DDR			DDRD
#define	KP_PIN			PIND
// WS2812
#define LEDS_PORT		PORTB
#define LEDS_DDR		DDRB
#define LEDS_PIN		0

/* ============== RAM ADDRESSES ============= */
// Offset for External RAM
#define RAM_OFFSET		0x8000
// 170 Leds max (x 3 bytes = 0x1FE)
#define RAM_LEDS_START	(RAM_OFFSET + 0x000)
#define RAM_LEDS_END	(RAM_OFFSET + 0x1FE)
#define RAM_LEDS_AMOUNT	(RAM_OFFSET + 0x1FF)
// 80 bytes of char buffer for LCD
#define RAM_LCD_START	(RAM_OFFSET + 0x200)
#define RAM_LCD_END		(RAM_OFFSET + 0x250)
#define RAM_LCD_CMD		(RAM_OFFSET + 0x251)
// Mask for global LED dimming
#define RAM_LEDS_DIM	(RAM_OFFSET + 0x252)
#define RAM_KP_LASTKEY	(RAM_OFFSET + 0x253)

#define RAM_VERSION_1	(RAM_OFFSET + 0x300)
#define RAM_VERSION_2	(RAM_OFFSET + 0x301)

// Interrupt registers
#define RAM_INT_SEND	(RAM_OFFSET + 0x3FE)
#define RAM_INT_GET		(RAM_OFFSET + 0x3FF)

#define MAX_LEDS		(RAM_LEDS_END - RAM_LEDS_START)
#define MAX_LCD_CHARS	(RAM_LCD_END - RAM_LCD_START)

/* ================ CMD CODES =============== */
#define CMD_LEDS_SEND	0x01
#define CMD_LCD_CHAR	0x02
#define CMD_LCD_CMD		0x03
#define CMD_LCD_CL_PR	0x04
#define CMD_LCD_POS		0x05

#define SATUS_KP_PRESS	0x01

/* ================= MACROS ================= */
/*	
	!WARNING! The data-lines on my custom PCB are flipped on the AVR side. 
	This means that these macros use the '__builtin_avr_insert_bits' macro to flip the data read/written from/to the DP-RAM.
	If you don't need this, replace '__builtin_avr_insert_bits (0x01234567, {value}, 0)' with '{value}'.
*/
#define PONTER_RAM(addr)		( ((volatile uint8_t *) addr) )
#define READ_RAM(addr)			( __builtin_avr_insert_bits (0x01234567, *PONTER_RAM(addr), 0) )
#define WRITE_RAM(addr, val)	{ *PONTER_RAM(addr) = __builtin_avr_insert_bits (0x01234567, val, 0); }

/* ================= STRUCTS ================ */
// Correct byte order!
struct cRGB { uint8_t g; uint8_t r; uint8_t b; };

/* ================= GLOBALS ================ */
struct cRGB LEDS[MAX_LEDS];
uint8_t LCD_BACKLIGHT = 0;

/* ================ FUNCTIONS =============== */

/* ---------------- inits ----------------
 * Parameters: N/a
 * Actions:
 *	-	Enable External Memory Interface
 *	-	Set `LCD_PORT` as output (with `LCD_MASK`)
 *	-	Set `KP_PORT`.[0->3] as output
 *	-	Set `KP_PORT`.[4->8] as input
 *	-	Set `LEDS_PORT`.`LEDS_PIN` as output
 *	-	Init & Clear LCD
 *  -	Backlight LCD ON
 *  -	LCD Cursor OFF
 */
void inline inits();

/* ---------------- readMatrix ----------------
 * Parameters:
 * -	matrix				Pin readout of the matrix port
 * Actions:
 * -	Convert matrix code to ACSII
 * Matrix layout:
 *		Bit | 5 | 6 | 7 | 8 
 *		----+---+---+---+---
 *		  1 | 1 | 2 | 3 | A 
 *		  2 | 4 | 5 | 6 | B 
 *		  3 | 7 | 8 | 9 | C 
 *		  4 | * | 0 | # | D 
 */
uint8_t readMatrix(uint8_t matrix);

/* ---------------- sendLEDS ----------------
 * Parameters:
 *	-	amountOfLeds		The amount of led information to send
 * Actions:
 *	-	Send LED data in `LEDS` to `LEDS_PORT`.`LEDS_PIN`
 *	-	Delay 50탎 for reset pulse
 * Prerequisites:
 *	-	`LEDS_PORT`.`LEDS_PIN` has been set as output
 */
void inline sendLEDS(uint16_t amountOfLeds);

/* ---------------- sendLCDNible ----------------
 * Parameters:
 *	-	data			Populate 4 lower bits
 *	-	rs				1 for character, 0 for instruction
 * Actions:
 *	-	Mask data with 0x0F
 *	-	Set data bit 7 (Backlight) to `LCD_BACKLIGHT`
 *	-	Set data bit 6 (RegisterSelect) to`rs`
 *	-	Set data bit 5 (Enable) to 1
 *	-	Set data bit 4 (INT4) to 1 (for Pull-up)
 *	-	Write data to `LCD_PORT`
 *	-	delay 1 탎
 *	-	Toggle bit 5 (Enable)
 *	-	delay 2 탎
 */
void sendLCDNible(uint8_t data, uint8_t rs);

/* ---------------- sendLCDInstructionByte ----------------
 * Parameters:
 *	-	data			Data to send to the instruction register
 *  Actions:
 *	-	Call sendLCDNible(data >> 4, 0)
 *	-	Call sendLCDNible(data, 0)
 *	-	delay 50탎
 */
void sendLCDInstructionByte(uint8_t data);

/* ---------------- sendLCDCharacterByte ----------------
 * Parameters:
 *	-	data			Data to send to the character register
 *  Actions:
 *	-	Call sendLCDNible(data >> 4, 1)
 *	-	Call sendLCDNible(data, 1)
 *	-	delay 50탎
 */
void sendLCDCharacterByte(uint8_t data);

/* ---------------- sendLCDBuffer ----------------
 * Parameters:
 *	-	*buffer			Pointer to character buffer
 *  Actions:
 *	-	For every character in the buffer or until `MAX_LCD_CHARS` is reached:
 *		-	Call sendLCDCharacterByte(data)
 */
void sendLCDBuffer(uint8_t * buffer);

/* ---------------- sendLCD ----------------
 * Parameters:
 *	-	option			0 = print; 1 = instruction; 2 = clear & print
 *  Actions:
 *	-	For every character in the buffer or until `MAX_LCD_CHARS` is reached:
 *		-	Call sendLCDCharacterByte(data)
 */
void sendLCD(uint8_t buffer);

#endif /* AVR_H_ */