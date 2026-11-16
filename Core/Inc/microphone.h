
#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <stdint.h>

// Number of microphone samples to collect
#define BUF_LEN 1024

// Stores the ADC readings
extern volatile uint16_t mic_buffer[BUF_LEN];

// Flag indicating capture is complete
extern volatile uint8_t mic_buffer_full;

// Initialize ADC, DMA, and TIM6
void microphone_init(void);

// Start collecting microphone samples
void microphone_start_capture(void);

#endif
