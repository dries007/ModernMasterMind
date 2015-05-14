AVR
===

This folder contains the source of the program used in the AVR ATmega128A

Because of an error in my PCB design, the databus lines are flipped. 
This is resolved in the MACROS section of the AVR.h file.

**  Don't forget to change the F_CPU if you are not running the AVR on 8MHz.    **

**  The Stack pointer should be set!    **

Software used: 
-   [Atmel Studio 6](http://www.atmel.com/microsite/atmel_studio6/)

Libraries used:
-   [light_ws2812](https://github.com/cpldcpu/light_ws2812/) Modified to fit in one file because of compiler issues.
