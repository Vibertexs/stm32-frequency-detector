
#include "stm32l4xx.h"
#include "microphone.h"

// Store 1024 microphone samples
volatile uint16_t mic_buffer[BUF_LEN];

// Flag indicating capture is complete
volatile uint8_t mic_buffer_full = 0;


// Initialize microphone sampling
void microphone_init(void)
{
    // Enable GPIOA clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // Configure PA0 as analog input
    GPIOA->MODER |= GPIO_MODER_MODE0;

    // Disable pull-up and pull-down resistors
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;

    // Connect PA0 to ADC
    GPIOA->ASCR |= GPIO_ASCR_ASC0;


    // ---------------- ADC1 ----------------

    // Enable ADC clock
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

    // Select HCLK as ADC clock
    ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE;
    ADC123_COMMON->CCR |= ADC_CCR_CKMODE_0;

    // Exit deep-power-down mode
    ADC1->CR &= ~ADC_CR_DEEPPWD;

    // Enable ADC voltage regulator
    ADC1->CR |= ADC_CR_ADVREGEN;

    // Wait for regulator stabilization
    for (volatile uint32_t i = 0; i < 1000; i++);

    // 12-bit resolution
    ADC1->CFGR &= ~ADC_CFGR_RES;

    // Right-aligned data
    ADC1->CFGR &= ~ADC_CFGR_ALIGN;

    // One conversion per sequence
    ADC1->SQR1 &= ~ADC_SQR1_L;

    // Select channel 5 (PA0)
    ADC1->SQR1 &= ~ADC_SQR1_SQ1;
    ADC1->SQR1 |= (5U << ADC_SQR1_SQ1_Pos);

    // Sample time: 247.5 ADC cycles
    ADC1->SMPR1 &= ~ADC_SMPR1_SMP5;
    ADC1->SMPR1 |= (6U << ADC_SMPR1_SMP5_Pos);

    // Select TIM6 TRGO as ADC trigger
    ADC1->CFGR &= ~ADC_CFGR_EXTSEL;
    ADC1->CFGR |= (13U << ADC_CFGR_EXTSEL_Pos);

    // Trigger ADC on rising edge
    ADC1->CFGR &= ~ADC_CFGR_EXTEN;
    ADC1->CFGR |= ADC_CFGR_EXTEN_0;

    // Enable DMA requests from ADC
    ADC1->CFGR |= ADC_CFGR_DMAEN;

    // Continue DMA requests across conversions
    ADC1->CFGR |= ADC_CFGR_DMACFG;

    // Select single-ended calibration
    ADC1->CR &= ~ADC_CR_ADCALDIF;

    // Start calibration
    ADC1->CR |= ADC_CR_ADCAL;

    // Wait for calibration to finish
    while (ADC1->CR & ADC_CR_ADCAL);

    // Clear ADC ready flag
    ADC1->ISR = ADC_ISR_ADRDY;

    // Enable ADC
    ADC1->CR |= ADC_CR_ADEN;

    // Wait until ADC is ready
    while (!(ADC1->ISR & ADC_ISR_ADRDY));


    // ---------------- DMA1 ----------------

    // Enable DMA1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    // Disable DMA channel before setup
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Select ADC1 request for DMA1 channel 1
    DMA1_CSELR->CSELR &= ~DMA_CSELR_C1S;

    // Read data from ADC1 data register
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;

    // Store data in microphone buffer
    DMA1_Channel1->CMAR = (uint32_t)mic_buffer;

    // Set number of transfers
    DMA1_Channel1->CNDTR = BUF_LEN;

    // Transfer from peripheral to memory
    DMA1_Channel1->CCR &= ~DMA_CCR_DIR;

    // Disable peripheral address increment
    DMA1_Channel1->CCR &= ~DMA_CCR_PINC;

    // Enable memory address increment
    DMA1_Channel1->CCR |= DMA_CCR_MINC;

    // Peripheral data size: 16 bits
    DMA1_Channel1->CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;

    // Memory data size: 16 bits
    DMA1_Channel1->CCR &= ~DMA_CCR_MSIZE;
    DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;

    // Normal DMA mode (not circular)
    DMA1_Channel1->CCR &= ~DMA_CCR_CIRC;

    // Enable transfer-complete interrupt
    DMA1_Channel1->CCR |= DMA_CCR_TCIE;

    // Enable DMA interrupt in NVIC
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);


    // ---------------- TIM6 ----------------

    // Enable TIM6 clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;

    // Stop TIM6 during configuration
    TIM6->CR1 &= ~TIM_CR1_CEN;

    // No prescaler
    TIM6->PSC = 0;

    // 48 MHz / 3000 = 16000 Hz
    TIM6->ARR = 2999;

    // Select update event as TRGO
    TIM6->CR2 &= ~TIM_CR2_MMS;
    TIM6->CR2 |= TIM_CR2_MMS_1;

    // Load the timer settings
    TIM6->EGR = TIM_EGR_UG;

    // Clear update flag
    TIM6->SR &= ~TIM_SR_UIF;
}


// Start a new microphone capture
void microphone_start_capture(void)
{
    // Stop TIM6
    TIM6->CR1 &= ~TIM_CR1_CEN;

    // Stop any previous ADC conversion sequence
    if (ADC1->CR & ADC_CR_ADSTART)
    {
        ADC1->CR |= ADC_CR_ADSTP;

        while (ADC1->CR & ADC_CR_ADSTP);
    }

    // Disable DMA channel
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Reset capture complete flag
    mic_buffer_full = 0;

    // Clear DMA interrupt flags
    DMA1->IFCR = DMA_IFCR_CGIF1;

    // Reset DMA transfer count
    DMA1_Channel1->CNDTR = BUF_LEN;

    // Reset DMA memory address
    DMA1_Channel1->CMAR = (uint32_t)mic_buffer;

    // Enable DMA channel
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    // Reset timer counter
    TIM6->CNT = 0;

    // Clear ADC overrun flag
    ADC1->ISR = ADC_ISR_OVR;

    // Arm ADC to wait for timer triggers
    ADC1->CR |= ADC_CR_ADSTART;

    // Start TIM6
    TIM6->CR1 |= TIM_CR1_CEN;
}


// DMA transfer-complete interrupt
void DMA1_Channel1_IRQHandler(void)
{
    // Check transfer-complete flag
    if (DMA1->ISR & DMA_ISR_TCIF1)
    {
        // Clear DMA interrupt flag
        DMA1->IFCR = DMA_IFCR_CGIF1;

        // Stop TIM6
        TIM6->CR1 &= ~TIM_CR1_CEN;

        // Signal that the buffer is full
        mic_buffer_full = 1;
    }
}
