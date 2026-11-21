# Checking and reading the ID of Nokia displays that operate via the SPI bus.

Arduino outputs the following data to the serial port:

~~~
=== 1. Identifier polling (ID) ===
CMD 0x04 (RDDID 24-bit): 0xE38166
CMD 0xDA (ID1 - Module): 0xE3
CMD 0xDB (ID2 - Ver)   : 0x81
CMD 0xDC (ID3 - Driver): 0x66
CMD 0xD3 (ID4 24-bit)  : 0x000000
CMD 0xD3 (ID4 32-bit)  : 0x00000000

=== 2. Polling of status and internal registers ===
CMD 0x09 (Status 32b)  : 0x80630000
CMD 0x0A (Power Mode)  : 0x98
CMD 0x0F (Self-Diag)   : 0xC0
CMD 0xFD (NV3029 check): 0x000000
CMD 0xEF (Vendor check): 0x000000

=== 3. Automatic verdict ===
-> Controller: Sitronix ST7735S with OEM firmware for the Nokia module (0xE3)

=== 4. Full dump of all live registers (0x00..0xFF) ===
CMD 0x04 -> NoDummy: 0x71C0B300 | Dummy: 0xE3816600
CMD 0x09 -> NoDummy: 0x00308000 | Dummy: 0x00610000
CMD 0x0A -> NoDummy: 0x08000000 | Dummy: 0x10000000
CMD 0x0C -> NoDummy: 0x06000000 | Dummy: 0x0C000000
CMD 0xDA -> NoDummy: 0xE3000000 | Dummy: 0xC6000000
CMD 0xDB -> NoDummy: 0x81000000 | Dummy: 0x02000000
CMD 0xDC -> NoDummy: 0x66000000 | Dummy: 0xCC000000
~~~

Example of a controller description by ID:
__ID byte decoding: 0x548366__

- 0x54 — official manufacturer code for Ilitek.
- 0x83 — controller/driver version (a revision preceding the 0x84 die revision).
- 0x66 — standard OEM firmware/module identifier for Nokia displays.


Pins for connecting the display to an Arduino:

LCD: |  Arduino:
:--- | ---:
SCK | 13
SDA | 11
CS |  10
RST |  9

