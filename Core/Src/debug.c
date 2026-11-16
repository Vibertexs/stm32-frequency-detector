
#include "stm32l4xx.h"
#include "debug.h"

#define UART_BAUD 115200U
#define CPU_FREQ 48000000U

// Initialize USART2 for serial communication
void debug_init(void)
{
    // Enable GPIOA and USART2 clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // Set PA2 to alternate function mode (TX)
    GPIOA->MODER &= ~GPIO_MODER_MODE2;
    GPIOA->MODER |= GPIO_MODER_MODE2_1;

    // Set PA3 to alternate function mode (RX)
    GPIOA->MODER &= ~GPIO_MODER_MODE3;
    GPIOA->MODER |= GPIO_MODER_MODE3_1;

    // Select alternate function 7 for PA2
    GPIOA->AFR[0] &= ~(0xFU << 8);
    GPIOA->AFR[0] |= (7U << 8);

    // Select alternate function 7 for PA3
    GPIOA->AFR[0] &= ~(0xFU << 12);
    GPIOA->AFR[0] |= (7U << 12);

    // Disable USART2 before configuration
    USART2->CR1 &= ~USART_CR1_UE;

    // Set baud rate to 115200
    USART2->BRR = (CPU_FREQ + UART_BAUD / 2U) / UART_BAUD;

    // Configure 8 data bits, no parity
    USART2->CR1 &= ~USART_CR1_M0;
    USART2->CR1 &= ~USART_CR1_M1;
    USART2->CR1 &= ~USART_CR1_PCE;

    // Configure 1 stop bit
    USART2->CR2 &= ~USART_CR2_STOP;

    // Enable transmitter
    USART2->CR1 |= USART_CR1_TE;

    // Enable receiver
    USART2->CR1 |= USART_CR1_RE;

    // Enable USART2
    USART2->CR1 |= USART_CR1_UE;
}

// Send one character through UART
void debug_putc(char c)
{
    // Wait until transmit register is empty
    while (!(USART2->ISR & USART_ISR_TXE));

    // Send character
    USART2->TDR = (uint8_t)c;
}

// Send a string through UART
void debug_print(const char *str)
{
    while (*str != '\0')
    {
        debug_putc(*str);
        str++;
    }
}

// Print an unsigned integer
void debug_print_uint(uint32_t num)
{
    char buffer[10];
    uint8_t i = 0;

    // Convert number into individual digits
    do
    {
        buffer[i] = '0' + (num % 10U);
        i++;
        num /= 10U;
    }
    while (num > 0U);

    // Print digits in the correct order
    while (i > 0U)
    {
        i--;
        debug_putc(buffer[i]);
    }
}

// Print a signed integer
void debug_print_int(int32_t num)
{
    if (num < 0)
    {
        debug_putc('-');
        debug_print_uint((uint32_t)(-(int64_t)num));
    }
    else
    {
        debug_print_uint((uint32_t)num);
    }
}
