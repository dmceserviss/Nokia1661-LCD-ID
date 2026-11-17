# Nokia1661_LCD_ID
Checking and reading the ID of Nokia displays that operate via the SPI bus.
Example of a controller description by ID:
ID byte decoding: 0x548366.
• 0x54 — official manufacturer code for Ilitek.
• 0x83 — controller/driver version (a revision preceding the 0x84 die revision).
• 0x66 — standard OEM firmware/module identifier for Nokia displays.
Pins for connecting the display to an Arduino:
LCD:       Arduino:
SCK        13
SDA        11
CS         10
RST        9

