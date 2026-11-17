
#include "lcd.h"

// LCD connections:
// PB0-PB7 = LCD D0-D7
// PC0 = RS
// PC1 = RW
// PC2 = E

// Clear the eight LCD data pins
static void lcd_clear_data(void)
{
    GPIOB->ODR &= ~0xFF;
}

// Send a command to the LCD
static void lcd_command(uint8_t data)
{
    // Clear old data
    lcd_clear_data();

    // Put command on PB0-PB7
    GPIOB->ODR |= data;

    // RS = 0 (command)
    GPIOC->ODR &= ~(1 << 0);

    // RW = 0 (write)
    GPIOC->ODR &= ~(1 << 1);

    HAL_Delay(1);

    // Enable high
    GPIOC->ODR |= (1 << 2);

    HAL_Delay(1);

    // Enable low
    GPIOC->ODR &= ~(1 << 2);

    HAL_Delay(1);
}

// Send one character to the LCD
static void lcd_write_char(uint8_t data)
{
    // RS = 1 (character)
    GPIOC->ODR |= (1 << 0);

    // RW = 0 (write)
    GPIOC->ODR &= ~(1 << 1);

    // Enable low while preparing the data
    GPIOC->ODR &= ~(1 << 2);

    // Clear old data
    lcd_clear_data();

    // Put character on PB0-PB7
    GPIOB->ODR |= data;

    HAL_Delay(1);

    // Enable high
    GPIOC->ODR |= (1 << 2);

    HAL_Delay(1);

    // Enable low
    GPIOC->ODR &= ~(1 << 2);

    HAL_Delay(1);
}

// Initialize LCD using 8-bit communication
void lcd_init(void)
{
    // Enable GPIOB clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // Enable GPIOC clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    // Configure PB0-PB7 as outputs
    GPIOB->MODER &= ~0xFFFF;
    GPIOB->MODER |= 0x5555;

    // Configure PC0-PC2 as outputs
    GPIOC->MODER &= ~0x3F;
    GPIOC->MODER |= 0x15;

    // Start with all LCD signals low
    lcd_clear_data();

    GPIOC->ODR &= ~(1 << 0);
    GPIOC->ODR &= ~(1 << 1);
    GPIOC->ODR &= ~(1 << 2);

    // Wait for LCD to power up
    HAL_Delay(100);

    // LCD initialization sequence
    lcd_command(0x30);
    HAL_Delay(30);

    lcd_command(0x30);
    HAL_Delay(10);

    lcd_command(0x30);
    HAL_Delay(10);

    // 8-bit mode, 2 lines, 5x8 characters
    lcd_command(0x38);

    // Display off
    lcd_command(0x08);

    // Clear display
    lcd_command(0x01);
    HAL_Delay(5);

    // Entry mode: cursor moves right
    lcd_command(0x06);

    // Display on, cursor off
    lcd_command(0x0C);
}

// Clear the LCD screen
void lcd_clear(void)
{
    lcd_command(0x01);
    HAL_Delay(2);
}

// Move the cursor to a row and column
void lcd_set_cursor(uint8_t row, uint8_t col)
{
    if (row > 1 || col > 15)
    {
        return;
    }

    uint8_t address;

    if (row == 0)
    {
        address = 0x80 + col;
    }
    else
    {
        address = 0xC0 + col;
    }

    lcd_command(address);
}

// Print a string to the LCD
void lcd_print(const char *text)
{
    while (*text != '\0')
    {
        lcd_write_char((uint8_t)*text);

        text++;
    }
}



// Print an unsigned integer on the LCD
void lcd_print_uint(uint32_t number)
{
    char digits[10];
    uint32_t count = 0;

    // Special case for zero
    if (number == 0)
    {
        lcd_write_char('0');
        return;
    }

    // Extract digits in reverse order
    while (number > 0)
    {
        digits[count] = '0' + (number % 10);

        number /= 10;
        count++;
    }

    // Print digits in the correct order
    while (count > 0)
    {
        count--;
        lcd_write_char(digits[count]);
    }
}
