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

#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "AVR.h"

/* ======= INTERRUPT SERVICE ROUTINES ======= */
// Interrupt Service Routine for INT4
ISR(INT4_vect)
{
	volatile uint8_t cmd = READ_RAM(RAM_INT_GET); // read interrupt address, also clears interrupt signal
	
	switch (cmd)
	{
		case CMD_LEDS_SEND: // Copy LED data from DPRAM into RAM (sets correct byte order) and clock out the data
		{
			uint8_t n = READ_RAM(RAM_LEDS_AMOUNT); // Amount of LEDS connected
			uint8_t dim = PORTD = READ_RAM(RAM_LEDS_DIM); // Global LED dimmer settings
			if (n > MAX_LEDS) n = MAX_LEDS; // Make sure that n <= MAX_LEDS to prevent data corruption
			uint16_t offset = 0; // used for DPRAM address offset from RAM_LEDS_START
			for (uint16_t i = 0; i < n; i++)
			{
				LEDS[i].r = READ_RAM(RAM_LEDS_START + (offset ++)) & dim; // R 
				LEDS[i].g = READ_RAM(RAM_LEDS_START + (offset ++)) & dim; // G 
				LEDS[i].b = READ_RAM(RAM_LEDS_START + (offset ++)) & dim; // B 
			}
			sendLEDS(n); // Clock out data
		}
		break;
		case CMD_LCD_POS: // Shortcut command to set LCD cursor position
		{
			sendLCDInstructionByte(READ_RAM(RAM_LCD_CMD) | 0b10000000);
		}
		break;
		case CMD_LCD_CMD: // Send LCD an insrtuction byte
		{
			sendLCDInstructionByte(READ_RAM(RAM_LCD_CMD));
		}
		break;
		case CMD_LCD_CL_PR: // Shortcut for clear & print
		{
			sendLCDInstructionByte(0x01);
			_delay_ms(10);
			// no break!
		}
		case CMD_LCD_CHAR: // Print char buffer (until max chars or 0x00)
		{
			for (uint8_t i = 0; i < MAX_LCD_CHARS; i++)
			{
				uint8_t c = READ_RAM(RAM_LCD_START + i);
				if (c == 0x00) break;
				sendLCDCharacterByte(c);
			}
		}
		break;
	}
	/* Interrupt detection debug code */
	#if DEBUG
		char buff[10];
		sprintf(buff, "I:0x%02X", cmd);
		sendLCDBuffer(buff);
	#endif
	sei();
}

/* ================== DEBUG STUFF ================== */
/* HANDLE DEBUG KEYPRESS HERE */
void debugKeypress(uint8_t key)
{
	static uint8_t b = 0; // buffer index pointer
	static char buffer[20]; // buffer (20 chars = 1 line)
	
	switch (key)
	{
		// SEND
		case '*':
		{
			sendLEDS(30);
			
			b = 0;
			buffer[b] = 0;
			sendLCDInstructionByte(0x01);
			_delay_ms(2);
			sprintf(buffer, "      0x%02X 0x%02X 0x%02X", LEDS[0].r, LEDS[0].g, LEDS[0].b);
			sendLCDBuffer(buffer);
			sendLCDInstructionByte(0x80); // 1e pos on lcd
			break;
		}
		// BACKSPACE
		case '#':
		{
			if (b != 0) b--;
			buffer[b] = 0;
			
			sendLCDInstructionByte(0x01);
			_delay_ms(2);
			
			sendLCDBuffer(buffer);
			break;
		}
		// SET RED
		case 'A':
		{
			uint8_t nr = atoi(buffer);
			for (uint8_t i = 0; i < 30; i++)
			{
				LEDS[i].r = nr;
			}
			b = 0;
			buffer[b] = 0;
			sendLCDInstructionByte(0x01);
			_delay_ms(2);
			break;
		}
		// SET GREEN
		case 'B':
		{
			uint8_t nr = atoi(buffer);
			for (uint8_t i = 0; i < 30; i++)
			{
				LEDS[i].g = nr;
			}
			b = 0;
			buffer[b] = 0;
			sendLCDInstructionByte(0x01);
			_delay_ms(2);
			break;
		}
		// SET BLUE
		case 'C':
		{
			uint8_t nr = atoi(buffer);
			for (uint8_t i = 0; i < 30; i++)
			{
				LEDS[i].b = nr;
			}
			b = 0;
			buffer[b] = 0;
			sendLCDInstructionByte(0x01);
			_delay_ms(2);
			break;
		}
		case 'D':
		{
			sendLCDInstructionByte(0x01);
			for (uint8_t i = 0; i < MAX_LCD_CHARS; i++)
			{
				uint8_t c = READ_RAM(RAM_LCD_START + i);
				if (c == 0x00) break;
				sendLCDCharacterByte(c);
			}
		}
		break;
		// NUMBER
		default:
		{
			buffer[b++] = key;
			buffer[b] = 0;
			sendLCDCharacterByte(key);
			break;
		}
	}
}

/* ================== MAIN ================== */
int main()
{
	inits(); // set ports & interrupt registers
	
	// debounce variables
	uint8_t prevKey = 0x00;
	uint16_t downTime = 0;
	uint16_t upTime = 0;
	
	while (1) // main program loop (aka keypad scanner)
	{
		for (uint8_t r = 0; r < 4; r++) // ROW loop
		{
			KP_PORT = 0x0F | (0b11101111 << r); // all pins HIGH, the row we want to read LOW; bit 0-4 always high because they are inputs (pull-up).
			uint8_t key = readMatrix(~KP_PIN); // Convert read port byte (inverted because pull-ups) 
			if (key == 0x00) // no key pressed
			{
				if (upTime++ > 100) // if no key was pressed for 100+ ms, reset debounce.
				{
					prevKey = 0x00; // Makes sure the next keypress will register instantly
					
					downTime = 0;
					upTime = 0;
				}
			}
			else // A key was pressed
			{
				if (prevKey != key || downTime++ > 500) // if the pressed key is different from the last one OR its been pressed for 500+ ms, acknowledge as a legitimate press
				{
					WRITE_RAM(RAM_KP_LASTKEY, key); // Store key in DPRAM for SC12
					WRITE_RAM(RAM_INT_SEND, SATUS_KP_PRESS); // Send interrupt to SC12
					
					#if DEBUG
					debugKeypress(key);
					#endif
					
					prevKey = key; // Store current key for debounce
					
					downTime = 0;
					upTime = 0;
				}
			}
			
			_delay_ms(1);
		}
	}
}

/* ================ FUNCTIONS =============== */
void inline inits()
{
	// Write SRE to 1 enables the External Memory Interface
	MCUCR = 0x80;
	
	// Magic numbers
	WRITE_RAM(RAM_VERSION_1, 42); 
	WRITE_RAM(RAM_VERSION_2, 0x42);
	
	// LCD Port Setup
	LCD_DDR = LCD_MASK;
	LCD_PORT = (uint8_t)~LCD_MASK;
	
	// Keypad Port Setup (bit 0-3 = in; bit 4-7 = out)
	KP_DDR = 0xF0;
	
	// LED port all output
	LEDS_DDR = 0xFF;
	
	// Enable falling edge interrupt INT4
	EICRB = 0x02;
	EIMSK = 0x10;
	
	// LCD init
	_delay_ms(100);
	
	// Set 4 bit mode
	sendLCDNible(0x02, 0);
	// 2-line mode, display on
	sendLCDInstructionByte(0x0C);
	_delay_ms(100);
	
	// Display ON/OFF Control
	sendLCDInstructionByte(0x0F);
	// Clear & home
	sendLCDInstructionByte(0x01);
	_delay_ms(20);
	// Entry mode Increment & Entire shift off
	sendLCDInstructionByte(0x06);
	
	// Clear any open interrupts.
	volatile uint8_t i = READ_RAM(RAM_INT_GET);
	// Global interrupts ON
	sei();
}

uint8_t inline readMatrix(uint8_t matrix)
{
	switch (matrix)
	{
		default: return 0x00;
		
		case 0b00010001: return '1';	// 0x11
		case 0b00010010: return '4';	// 0x12
		case 0b00010100: return '7';	// 0x13
		case 0b00011000: return '*';	// 0x18
		
		case 0b00100001: return '2';	// 0x21
		case 0b00100010: return '5';	// 0x22
		case 0b00100100: return '8';	// 0x24
		case 0b00101000: return '0';	// 0x28
		
		case 0b01000001: return '3';	// 0x41
		case 0b01000010: return '6';	// 0x42
		case 0b01000100: return '9';	// 0x44
		case 0b01001000: return '#';	// 0x48
		
		case 0b10000001: return 'A';	// 0x81
		case 0b10000010: return 'B';	// 0x82
		case 0b10000100: return 'C';	// 0x84
		case 0b10001000: return 'D';	// 0x88
	}
}

/* ########################################## START SECTION WS2812 DRIVER ########################################## 
 * Original Source:		https://github.com/cpldcpu/light_ws2812/
 * Original Author:		Tim (cpldcpu@gmail.com) 
 * Original License:	GNU GPL V2 (https://github.com/cpldcpu/light_ws2812/blob/master/License.txt)
 *		This license still applies to everything between the "WS2812 DRIVER" section lines.
 *
 * Modifications by Dries007:
 *	-	Changed configuration
 *	-	Merged library into single set of source and header files
 */
void inline sendLEDS(uint16_t leds)
{
	uint16_t datlen = leds + leds + leds;	// 3 colors!
	uint8_t * data = (uint8_t *) LEDS;		// Type cast
	
	uint8_t reg_prev = SREG;	// Save interrupt status
	cli();						// We can't be interrupted!
	
	uint8_t maskhi = _BV(LEDS_PIN);
	uint8_t masklo	= ~maskhi&LEDS_PORT;	// Low mask
	uint8_t maskhi |=         LEDS_PORT;	// High mask
	
	uint8_t curbyte, ctr; // used in ASM
	while (datlen--)
	{
		curbyte = *data ++; // Grab byte
		
		asm volatile(
		"       ldi   %0,8  \n\t"	// Write 8 (00001000) to Loop counter (%0)							Timing table:
		"loop%=:            \n\t"	// Loop entry point (%= is a unique number on each asm statement)
		"       out   %2,%3 \n\t"   // Write High mask (%3) to LED_PORT (%2)							'1' [01] '0' [01] - re
		#if (w1_nops&1)				// w1 nops for timing
		w_nop1
		#endif
		#if (w1_nops&2)
		w_nop2
		#endif
		#if (w1_nops&4)
		w_nop4
		#endif
		#if (w1_nops&8)
		w_nop8
		#endif
		#if (w1_nops&16)
		w_nop16
		#endif
		"       sbrs  %1,7  \n\t"	// Skip next instruction if bit 7 of Data (%1) is set				'1' [03] '0' [02]
		"       out   %2,%4 \n\t"	// Write Low mask (%4) to LED_PORT (%2)								'1' [--] '0' [03] - fe-low
		"       lsl   %1    \n\t"	// Shift Data (%1) left												'1' [04] '0' [04]
		#if (w2_nops&1)				// w2 nops for timing
		w_nop1
		#endif
		#if (w2_nops&2)
		w_nop2
		#endif
		#if (w2_nops&4)
		w_nop4
		#endif
		#if (w2_nops&8)
		w_nop8
		#endif
		#if (w2_nops&16)
		w_nop16
		#endif
		"       out   %2,%4 \n\t"	// Write Low mask (%4) to LED_PORT (%2)								'1' [+1] '0' [+1] - fe-high
		#if (w3_nops&1)				// w3 nops for timing
		w_nop1
		#endif
		#if (w3_nops&2)
		w_nop2
		#endif
		#if (w3_nops&4)
		w_nop4
		#endif
		#if (w3_nops&8)
		w_nop8
		#endif
		#if (w3_nops&16)
		w_nop16
		#endif
		"       dec   %0    \n\t"	// Decrement Loop counter (%0) (Also sets Z if 0x00)				'1' [+2] '0' [+2]
		"       brne  loop%=\n\t"	// Jump to Loop entry point if Z is set.							'1' [+3] '0' [+4]
		// %0 = 8 bit loop counter
		:	"=&d" (ctr)
		:	"r" (curbyte), "I" (_SFR_IO_ADDR(LEDS_PORT)), "r" (maskhi), "r" (masklo)
		//	%s1 = Data		%2 = LEDS_PORT				%3 = high mask	%4 = low mask
		);
	}
	SREG = sreg_prev; // Restore interrupt status
	_delay_us(50); // Reset delay
}
/* ########################################## END SECTION WS2812 DRIVER ########################################## */

void sendLCDNible(uint8_t data, uint8_t rs)
{
						data &= 0b00001111;	// Mask out fist 4 bits
	if (LCD_BACKLIGHT)	data |= 0b10000000;	// Mask in LCD_BACKLIGHT if required (pin 7)s
	if (rs)				data |= 0b01000000;	// Mask in register select
						data |= 0b00010000; // Bit 4 => 1, its the interrput pin, its on pull-up!
	LCD_PORT = data;	// Set Data
	_delay_us(10);		// Small delay, data needs to be valid BEFORE enable
	LCD_PORT ^= 0b00100000; // Toggle enable
	_delay_us(50);		// Larger delay, LCD needs time to process
}

void sendLCDInstructionByte(uint8_t data)
{
	sendLCDNible(data >> 4, 0); // rs = 0 -> instruction
	sendLCDNible(data, 0);
}

void sendLCDCharacterByte(char data)
{
	sendLCDNible(data >> 4, 1); // rs = 1 -> data
	sendLCDNible(data, 1);
}

void sendLCDBuffer(char * buffer)
{
	for (uint8_t i = 0; i < MAX_LCD_CHARS; i++) // max chars
	{
		if (buffer[i] == 0x00) break; // End on 0x00
		sendLCDCharacterByte(buffer[i]);
	}
}
