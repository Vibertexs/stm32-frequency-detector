
/* USER CODE BEGIN Header */
/**
  * @file    : main.c
  * @brief   : STM32 Musical Frequency Detector
  *
  * Collects microphone samples using ADC and DMA.
  * Uses CMSIS-DSP FFT to detect audio frequencies.
  * Converts detected frequencies into musical notes.
  * Displays results using UART and LCD1602.
  * Uses DAC1 to adjust LCD contrast.
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
#include "debug.h"
#include "microphone.h"
#include "notes.h"
#include "lcd.h"
/* USER CODE END Includes */

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN 0 */

// Minimum peak-to-peak amplitude required for detection
#define SILENCE_THRESHOLD 500

// DAC value controlling LCD contrast
// 800 produces approximately 0.64V with a 3.3V reference
#define LCD_CONTRAST 800

// Initialize the FFT
int frequency_init(void);

// Detect frequency in tenths of a Hz
uint32_t frequency_detect(volatile uint16_t *samples);

// Initialize DAC1 channel 1 on PA4
void contrast_init(void)
{
    // Enable GPIOA clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // Configure PA4 in analog mode
    GPIOA->MODER |= (3U << 8);

    // Disable pull-up and pull-down on PA4
    GPIOA->PUPDR &= ~(3U << 8);

    // Enable DAC1 peripheral clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;

    // Set DAC output value
    DAC1->DHR12R1 = LCD_CONTRAST;

    // Enable DAC channel 1
    DAC1->CR |= DAC_CR_EN1;
}

/* USER CODE END 0 */

int main(void)
{
    // Initialize STM32 hardware
    HAL_Init();

    // Configure CPU clock to 48 MHz
    SystemClock_Config();

    // Initialize GPIO
    MX_GPIO_Init();

    /* USER CODE BEGIN 2 */

    // Initialize UART communication
    debug_init();

    // Initialize microphone ADC, DMA, and TIM6
    microphone_init();

    // Generate LCD contrast voltage on PA4
    contrast_init();

    // Initialize the LCD
    lcd_init();

    // Display startup message
    lcd_clear();

    lcd_set_cursor(0, 0);
    lcd_print("STM32 Tuner");

    lcd_set_cursor(1, 0);
    lcd_print("Waiting...");

    // Print startup message over UART
    debug_print("Microphone POC started!\r\n");

    // Initialize the 1024-point FFT
    if (frequency_init() == 1)
    {
        debug_print("FFT initialized successfully!\r\n");
    }
    else
    {
        debug_print("FFT initialization failed!\r\n");

        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("FFT Init Failed");

        // Stop if FFT initialization fails
        while (1)
        {
        }
    }

    // Count completed microphone captures
    uint32_t capture_count = 0;

    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */

    while (1)
    {
        // Start microphone capture
        microphone_start_capture();

        // Wait until 1024 samples are collected
        while (mic_buffer_full == 0);

        // Variables for microphone statistics
        uint16_t min = 4095;
        uint16_t max = 0;
        uint32_t sum = 0;
        uint32_t avg = 0;
        uint16_t pp = 0;

        // Go through all 1024 samples
        for (uint32_t i = 0; i < BUF_LEN; i++)
        {
            uint16_t sample = mic_buffer[i];

            // Find minimum ADC value
            if (sample < min)
            {
                min = sample;
            }

            // Find maximum ADC value
            if (sample > max)
            {
                max = sample;
            }

            // Add sample to total
            sum += sample;
        }

        // Calculate average ADC value
        avg = sum / BUF_LEN;

        // Calculate peak-to-peak amplitude
        pp = max - min;

        // Check whether the signal is loud enough
        if (pp > SILENCE_THRESHOLD)
        {
            // Detect frequency using FFT
            // Result is in tenths of a Hz
            uint32_t detected_frequency = frequency_detect(mic_buffer);

            // Variables for musical note
            const char *note_name;
            int32_t octave;

            // Convert frequency into a musical note
            notes_detect(detected_frequency, &note_name, &octave);

            // Update LCD with detected frequency and note
            lcd_clear();

            // First row: frequency
            lcd_set_cursor(0, 0);
            lcd_print("Freq: ");
            lcd_print_uint(detected_frequency / 10);
            lcd_print(".");
            lcd_print_uint(detected_frequency % 10);
            lcd_print(" Hz");

            // Second row: musical note
            lcd_set_cursor(1, 0);
            lcd_print("Note: ");
            lcd_print(note_name);
            lcd_print_uint((uint32_t)octave);

            // Print detected frequency over UART
            debug_print("Frequency: ");
            debug_print_uint(detected_frequency / 10);
            debug_print(".");
            debug_print_uint(detected_frequency % 10);
            debug_print(" Hz\r\n");

            // Print detected musical note over UART
            debug_print("Note: ");
            debug_print(note_name);
            debug_print_int(octave);
            debug_print("\r\n");
        }
        else
        {
            // Signal is below the threshold
            debug_print("No sound\r\n");

            // Display silence message
            lcd_clear();

            lcd_set_cursor(0, 0);
            lcd_print("No sound");

            lcd_set_cursor(1, 0);
            lcd_print("Waiting...");
        }

        // Print minimum ADC reading
        debug_print("min=");
        debug_print_uint(min);

        // Print maximum ADC reading
        debug_print(" max=");
        debug_print_uint(max);

        // Print average ADC reading
        debug_print(" avg=");
        debug_print_uint(avg);

        // Print peak-to-peak amplitude
        debug_print(" pp=");
        debug_print_uint(pp);

        debug_print("\r\n");

        // Count completed captures
        capture_count++;

        // Every 10 captures, print 32 raw ADC samples
        if (capture_count % 10 == 0)
        {
            debug_print("samples=");

            for (uint32_t i = 0; i < 32; i++)
            {
                debug_print_uint(mic_buffer[i]);

                if (i < 31)
                {
                    debug_print(",");
                }
            }

            debug_print("\r\n");
        }

        // Wait 500 milliseconds
        HAL_Delay(500);
    }

    /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure voltage scaling
    if (HAL_PWREx_ControlVoltageScaling(
            PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    // Configure MSI oscillator
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    // Configure system clock and bus clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK;
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure PA2 and PA3 for USART2
    GPIO_InitStruct.Pin = USART_TX_Pin;
    GPIO_InitStruct.Pin |= USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}

#endif
