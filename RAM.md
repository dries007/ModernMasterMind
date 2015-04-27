RAM
===

LEFT port = SC12

RIGHT port = AVR

Memory map
----------

Addresses       | Function
----------------|--------------------------
0x000 -> 0x1FE  | RGB Data for max 170 LEDS
0x1FF           | Amounts of LEDs
0x200 -> 0x250  | Character buffer LCD (80 char)
0x251           | LCD Command
0x252           | LEDs dim mask
0x253           | Keypad last pressed key
0x254 -> 0x3FD  | ** Available **
0x3FE           | Interrupt SC12 -> AVR
0x3FF           | Interrupt AVR -> SC12

Commands
--------

Interrupt SC12 -> AVR

CMD  | Function
-----|------- 
0x00 | Do nothing
0x01 | Send LEDs
0x02 | Send LCD Char Buffer
0x03 | Send LCD Command
0x04 | Clear & Print LCD
