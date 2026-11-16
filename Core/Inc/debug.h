
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

// Initialize USART2 for serial communication
void debug_init(void);

// Send one character
void debug_putc(char c);

// Send a string
void debug_print(const char *str);

// Print an unsigned integer
void debug_print_uint(uint32_t num);

// Print a signed integer
void debug_print_int(int32_t num);

#endif
