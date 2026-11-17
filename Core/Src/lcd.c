#include "lcd.h"

// PB0-7 data pins for D0-7
// RS = PC0, RW = PC1, E = PC2

static void clr_data(void)
{
    GPIOB->ODR &= ~(0xFF);
}

void lcd_command(uint8_t data)
{
    clr_data();
    GPIOB->ODR |= data;
    GPIOC->ODR &= ~(1 << 0); // Clear RS
    GPIOC->ODR &= ~(1 << 1); // Clear RW
    HAL_Delay(1);
    GPIOC->ODR |= (1 << 2);  // E High
    HAL_Delay(1);
    GPIOC->ODR &= ~(1 << 2); // E Low
}

void lcd_wr_char(uint8_t input)
{
    GPIOC->ODR |= (1 << 0);  // Set RS (data mode)
    GPIOC->ODR &= ~(1 << 1); // Clear RW
    GPIOC->ODR |= (1 << 2);  // E High
    HAL_Delay(1);
    clr_data();
    GPIOB->ODR |= (0xFF & input);
    GPIOC->ODR &= ~(1 << 2); // E Low
}

void lcd_wr_word(char *str)
{
    while (*str != '\0')
    {
        lcd_wr_char((uint8_t)*str);
        str++;
    }
}

void lcd_init(void)
{
    // Enable clocks for GPIOB and GPIOC
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    clr_data();

    // Configure PB0-PB7 as GPIO Output
    GPIOB->MODER &= ~(0xFFFF);
    GPIOB->MODER |= (0x5555);

    // Configure PC0, PC1, PC2 as GPIO Output
    GPIOC->ODR &= ~(0x7);
    GPIOC->MODER &= ~(0x3F);
    GPIOC->MODER |= (0x15);
    HAL_Delay(100);

    // Initial commands sequence for HD44780
    lcd_command(0x30);
    HAL_Delay(30);
    lcd_command(0x30);
    HAL_Delay(10);
    lcd_command(0x30);
    HAL_Delay(10);

    lcd_command(0x38); // 8-bit mode, 2 lines
    lcd_command(0x10); // Set cursor movement
    lcd_command(0x0C); // Display ON, cursor OFF
    lcd_command(0x06); // Entry mode set
    lcd_clear();
    lcd_command(0x0F); // Display ON, cursor blink
}

void lcd_set_cursor(uint8_t row)
{
    if (row == 0)
    {
        lcd_command(0x80); // Line 1 address
    }
    else if (row == 1)
    {
        lcd_command(0xC0); // Line 2 address
    }
}

void lcd_clear(void)
{
    lcd_command(0x01);
    HAL_Delay(2);
}