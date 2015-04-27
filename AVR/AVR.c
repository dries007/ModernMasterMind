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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "AVR.h"

/* ======= INTERRUPT SERVICE ROUTINES ======= */
// Interrupt Service Routine for INT4
ISR(INT4_vect)
{
	cli();
	volatile uint8_t cmd = READ_RAM(RAM_INT_GET);
	
	switch (cmd)
	{
		case CMD_LEDS_SEND:
		{
			uint8_t n = READ_RAM(RAM_LEDS_AMOUNT);
			uint8_t dim = PORTD = READ_RAM(RAM_LEDS_DIM);
			if (n > MAX_LEDS) n = MAX_LEDS;
			uint16_t offset = 0;
			for (uint16_t i = 0; i < n; i++)
			{
				LEDS[i].r = READ_RAM(RAM_LEDS_START + (offset ++)) & dim;
				LEDS[i].g = READ_RAM(RAM_LEDS_START + (offset ++)) & dim;
				LEDS[i].b = READ_RAM(RAM_LEDS_START + (offset ++)) & dim;
			}
			sendLEDS(n);
		}
		break;
		case CMD_LCD_POS:
		{
			sendLCDInstructionByte(READ_RAM(RAM_LCD_CMD) | 0b10000000);
		}
		break;
		case CMD_LCD_CMD:
		{
			sendLCDInstructionByte(READ_RAM(RAM_LCD_CMD));
		}
		break;
		case CMD_LCD_CL_PR:
		{
			sendLCDInstructionByte(0x01);
			_delay_ms(10);
			// no break!
		}
		case CMD_LCD_CHAR:
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

/* ================== MAIN ================== */
/* HANDLE DEBUG KEYPRESS HERE */
void debugKeypress(uint8_t key)
{
	static uint8_t b = 0;
	static char buffer[20];
	
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

int main()
{
	inits();
	
	uint8_t prevKey = 0x00;
	uint16_t downTime = 0;
	uint16_t upTime = 0;
	
	while (1)
	{
		for (uint8_t r = 0; r < 4; r++)
		{
			KP_PORT = 0x0F | (0xEF << r);
			uint8_t key = readMatrix(~KP_PIN);
			if (key == 0x00)
			{
				if (upTime++ > 100)
				{
					prevKey = 0x00;
					
					downTime = 0;
					upTime = 0;
				}
			}
			else
			{
				if (prevKey != key || downTime++ > 500)
				{
					WRITE_RAM(RAM_KP_LASTKEY, key);
					WRITE_RAM(RAM_INT_SEND, SATUS_KP_PRESS);
					
					#if DEBUG
					debugKeypress(key);
					#endif
					
					prevKey = key;
					
					downTime = 0;
					upTime = 0;
				}
			}
		}
		
		_delay_ms(10);
	}
}

/* ================ FUNCTIONS =============== */
void inline inits()
{
	// Write SRE to 1 enables the External Memory Interface
	MCUCR = 0x80;
	
	//Magic numbers
	WRITE_RAM(RAM_VERSION_1, 42); 
	WRITE_RAM(RAM_VERSION_2, 0x42);
	
	// LCD Port Setup
	LCD_DDR = LCD_MASK;
	LCD_PORT = ~LCD_MASK;
	
	// Keypad Port Setup
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
	volatile i = READ_RAM(RAM_INT_GET);
	// Global interrupts ON
	sei();
}

void sendLCDNible(uint8_t data, uint8_t rs)
{
						data &= 0b00001111;
	if (LCD_BACKLIGHT)	data |= 0b10000000;
	if (rs)				data |= 0b01000000;
						data |= 0b00110000; // Bit 5 & 4 => 1
	LCD_PORT = data;  // Set Data
	_delay_us(10);
	LCD_PORT ^= 0x20; // Toggle enable
	_delay_us(50);
}

void sendLCDInstructionByte(uint8_t data)
{
	sendLCDNible(data >> 4, 0);
	sendLCDNible(data, 0);
}

void sendLCDCharacterByte(uint8_t data)
{
	sendLCDNible(data >> 4, 1);
	sendLCDNible(data, 1);
}

void sendLCDBuffer(uint8_t * buffer)
{
	for (uint8_t i = 0; i < MAX_LCD_CHARS; i++)
	{
		if (buffer[i] == 0x00) break;
		sendLCDCharacterByte(buffer[i]);
	}
}

uint8_t readMatrix(uint8_t matrix)
{
	switch (matrix)
	{
		default: return 0x00;
		case 0x11: return '1';
		case 0x12: return '4';
		case 0x14: return '7';
		case 0x18: return '*';
		
		case 0x21: return '2';
		case 0x22: return '5';
		case 0x24: return '8';
		case 0x28: return '0';
		
		case 0x41: return '3';
		case 0x42: return '6';
		case 0x44: return '9';
		case 0x48: return '#';
		
		case 0x81: return 'A';
		case 0x82: return 'B';
		case 0x84: return 'C';
		case 0x88: return 'D';
	}
}

/* ########################################## WS2812 DRIVER ########################################## 
 * Original Source:		https://github.com/cpldcpu/light_ws2812/
 * Original Author:		Tim (cpldcpu@gmail.com) 
 * Original License:	GNU GPL V2 (https://github.com/cpldcpu/light_ws2812/blob/master/License.txt)
 *		This license still applies to everything between the "WS2812 DRIVER" section lines.
 *
 * Modifications by Dries007:
 *	-	Changed configuration
 *	-	Merged library into 1 file
 */

// Timing in ns
#define w_zeropulse   350
#define w_onepulse    900
#define w_totalperiod 1250

// Fixed cycles used by the inner loop
#define w_fixedlow    2
#define w_fixedhigh   4
#define w_fixedtotal  8   

// Insert NOPs to match the timing, if possible
#define w_zerocycles    (((F_CPU/1000)*w_zeropulse          )/1000000)
#define w_onecycles     (((F_CPU/1000)*w_onepulse    +500000)/1000000)
#define w_totalcycles   (((F_CPU/1000)*w_totalperiod +500000)/1000000)

// w1 - nops between rising edge and falling edge - low
#define w1 (w_zerocycles-w_fixedlow)
// w2   nops between fe low and fe high
#define w2 (w_onecycles-w_fixedhigh-w1)
// w3   nops to complete loop
#define w3 (w_totalcycles-w_fixedtotal-w1-w2)

#if w1>0
  #define w1_nops w1
#else
  #define w1_nops  0
#endif

// The only critical timing parameter is the minimum pulse length of the "0"
// Warn or throw error if this timing can not be met with current F_CPU settings.
#define w_lowtime ((w1_nops+w_fixedlow)*1000000)/(F_CPU/1000)
#if w_lowtime>550
   #error "WS2812 DRIVER: Sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif w_lowtime>450
   #warning "WS2812 DRIVER: The timing is critical and may only work on WS2812B, not on WS2812(S)."
   #warning "Please consider a higher clockspeed, if possible"
#endif   

#if w2>0
#define w2_nops w2
#else
#define w2_nops  0
#endif

#if w3>0
#define w3_nops w3
#else
#define w3_nops  0
#endif

#define w_nop1  "nop      \n\t"
#define w_nop2  "rjmp .+0 \n\t"
#define w_nop4  w_nop2 w_nop2
#define w_nop8  w_nop4 w_nop4
#define w_nop16 w_nop8 w_nop8

void inline ws2812_sendarray_mask(uint8_t *data,uint16_t datlen,uint8_t maskhi)
{
	uint8_t curbyte,ctr,masklo;
	uint8_t sreg_prev;
	
	masklo	=~maskhi&LEDS_PORT; // Low mask
	maskhi |=        LEDS_PORT;	// High mask
	sreg_prev=SREG;	//Save interrupt status
	cli();	// We can't be interrupted!

	while (datlen--)
	{
		curbyte=*data++; // Grab byte
		
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
	
	SREG=sreg_prev; // Restore interrupt status
}

void inline sendLEDS(uint16_t leds)
{
	LEDS_DDR |= _BV(LEDS_PIN); // Enable DDR
	ws2812_sendarray_mask((uint8_t*)LEDS, leds + leds + leds, _BV(LEDS_PIN)); // Clock out data
	_delay_us(50); // Reset delay
}

/* ########################################## WS2812 DRIVER ########################################## */
