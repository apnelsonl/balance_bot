#ifndef LCD_H_
#define LCD_H_

void lcd_write_nibble(char n, char rs, unsigned int t);

void lcd_write_byte(char n, char rs, unsigned int t);

void initialize_LCD(void);

void lcd_write_string(char *str);

void lcd_hex(int data);


#endif /* LCD_H_ */
