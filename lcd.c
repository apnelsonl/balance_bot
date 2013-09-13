#include <msp430g2553.h>
#include "delay.h"
#include "lcd.h"

void lcd_write_nibble(char n, char rs, unsigned int t) {
	rs <<= 5;
	P1OUT |= rs|0x10|(0x0f & n); // 0x10 for high E
	delay_usec(1); // keep E pulse high a little while
	P1OUT &= ~0x10; // make E go to 0
	delay_usec(t);
}

// high nibble then low nibble
void lcd_write_byte(char n, char rs, unsigned int t) {
	lcd_write_nibble(n >> 4, rs, 1);
	lcd_write_nibble(n, rs, t);
}

void initialize_LCD(void) {
	int i;
	for (i=0; i<100; i++) // delay 100ms to allow LCD powerup
		delay_usec(1000);

	// The first parameter in each call is the byte to be sent (as two nibbles),
	// the second parameter is the rs value (rs=0 indicates an instruction),
	// and the 3rd parameter is the time to delay after sending both nibbles (usec).
	// These commands are all fast (~40 us) except for "clear display" (2 ms)
	lcd_write_byte(0x20,0,50); // 2 lines, 5x8 dots
	lcd_write_byte(0x0C,0,50); // display on, no cursor, no blinking
	lcd_write_byte(0x14,0,50); // shift cursor right
	lcd_write_byte(0x01,0,2000); // clear display and cursor home

}

void lcd_write_string(char *str) {
	// clear lcd
	lcd_write_byte(0x01,0,2000);
	// write string
	int i;
	for(i = 0; i < 40 && str[i]; ++i)
		lcd_write_byte(str[i], 1, 50);
}

void lcd_hex(int data) {
	char nibbles[4] = {0,0,0,0};
	int i = 0;
	for(i = 0; i < 4; ++i) {
		nibbles[i] = data & 0x000F;
		if(nibbles[i] < 10)
			nibbles[i] = nibbles[i] + '0';
		else
			nibbles[i] = nibbles[i] + 'A' - 10;
		data >>= 4;
	}

	for(i=3; i >= 0; --i)
		lcd_write_byte(nibbles[i], 1, 50);

	lcd_write_byte(' ', 1, 50);
}
