
#ifndef LCD_H
#define LCD_H

#include "main.h"

// Initialize the LCD in 4-bit mode
void lcd_init(void);

// Clear the LCD screen
void lcd_clear(void);

// Move the cursor to a row and column
// Row: 0 or 1
// Column: 0 through 15
void lcd_set_cursor(uint8_t row, uint8_t col);

// Display a string on the LCD
void lcd_print(const char *text);

// Print an unsigned integer on the LCD
void lcd_print_uint(uint32_t number);

#endif
