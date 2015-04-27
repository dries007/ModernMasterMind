AVR
===

Gate - Function table:

Gate  | 0       | 1       | 2       | 3       | 4       | 5        | 6        | 7
------|---------|---------|---------|---------|---------|----------|----------|---------
A*    | AD0     | AD1     | AD2     | AD3     | AD4     | AD5      | AD6      | AD7
B     | LEDS    | nc      | nc      | nc      | nc      | nc       | nc       | nc
C*    | A8      | A9      | nc      | nc      | nc      | nc       | nc       | A15
D     | KP R1   | KP R2   | KP R3   | KP R4   | KP C1   | KP C2    | KP C3    | KP C4
E     | LCD DB4 | LCD DB5 | LCD DB6 | LCD DB7 | INT4*   | LCD E    | LCD RS   | LCD BG
F     | ADC0    | ADC1    | ADC2    | ADC3    | ADC4    | ADC5     | ADC6     | ADC7

Gate  | 7       | 6       | 5       | 4       | 3       | 2        | 1        | 0
------|---------|---------|---------|---------|---------|----------|----------|----------
A*    | AD7     | AD6     | AD5     | AD4     | AD3     | AD2      | AD1      | AD0
B     | nc      | nc      | nc      | nc      | nc      | nc       | nc       | LEDS
C*    | A15     | nc      | nc      | nc      | nc      | nc       | A8       | A9
D     | KP C4   | KP C3   | KP C2   | KP C1   | KP R4   | KP R3    | KP R2    | KP R1
E     | LCD BG  | LCD RS  | LCD E   | INT4*   | LCD DB7 | LCD DB6  | LCD DB5  | LCD DB4
F     | ADC7    | ADC6    | ADC5    | ADC4    | ADC3    | ADC2     | ADC1     | ADC0


\* = Not available on external header!

nc = Not in use / Not connected

SC12
====

Pin - Function table:

Pins          | Function
--------------|-----------------
1 -> 4        | UART 0
5, 6, 25, 26  | UART 1
7             | INT0
9 -> 15       | Address/Data Bus
17            | Reset / LED blink
18 -> 21      | Ethernet
22            | Read (RD)
23            | Write (WR)
24            | Address Latch (ALE)
27            | Chip Select (PCS#1)
28            | GPIO2 (A8)
29            | GPIO3 (A9)
30, 31        | I²C
