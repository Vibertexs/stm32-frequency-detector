#ifndef LCD_H
#define LCD_H

#include "stm32l4xx_hal.h"

// Initializes GPIO pins and sends startup sequence to the LCD screen
void lcd_init(void);

// Sends a command byte to the LCD
void lcd_command(uint8_t data);

// Writes a single ASCII character to the LCD display
void lcd_wr_char(uint8_t input);

// Writes a null-terminated string to the LCD display
void lcd_wr_word(char *str);

// Moves cursor to row 0 or row 1
void lcd_set_cursor(uint8_t row);

// Clears LCD display screen
void lcd_clear(void);

#endif /* LCD_H */