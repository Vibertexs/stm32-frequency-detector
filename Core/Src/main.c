
/* USER CODE BEGIN Header */
/**
  * @file    : main.c
  * @brief   : Audio Frequency Detector
  *
  * Reads microphone samples using ADC and DMA.
  * Uses CMSIS-DSP FFT to detect the strongest frequency.
  * Prints results over UART to Tera Term.
  */
/* USER CODE END Header */

#include "main.h"
#include "lcd.h"

/* USER CODE BEGIN Includes */
#include "debug.h"
#include "microphone.h"
/* USER CODE END Includes */

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN 0 */

// Initialize the FFT
int frequency_init(void);

// Detect the strongest frequency from microphone samples
uint32_t frequency_detect(volatile uint16_t *samples);

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

    // Print startup message
    debug_print("Microphone POC started!\r\n");

    // Initialize the 1024-point FFT
    if (frequency_init() == 1)
    {
        debug_print("FFT initialized successfully!\r\n");
    }
    else
    {
        debug_print("FFT initialization failed!\r\n");

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

        // Detect the strongest frequency using the FFT
        uint32_t detected_frequency = frequency_detect(mic_buffer);

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

            // Find minimum
            if (sample < min)
            {
                min = sample;
            }

            // Find maximum
            if (sample > max)
            {
                max = sample;
            }

            // Add sample to total
            sum += sample;
        }

        // Calculate average
        avg = sum / BUF_LEN;

        // Calculate peak-to-peak
        pp = max - min;

        // Print detected frequency
        debug_print("Frequency: ");
        debug_print_uint(detected_frequency);
        debug_print(" Hz\r\n");

        // Print minimum
        debug_print("min=");
        debug_print_uint(min);

        // Print maximum
        debug_print(" max=");
        debug_print_uint(max);

        // Print average
        debug_print(" avg=");
        debug_print_uint(avg);

        // Print peak-to-peak
        debug_print(" pp=");
        debug_print_uint(pp);

        // Move to next line
        debug_print("\r\n");

        // Count completed captures
        capture_count++;

        // Every 10 captures, print 32 raw samples
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
