
#include "arm_math.h"
#include <stdint.h>

#define FFT_SIZE 1024
#define SAMPLE_RATE 16000

// Configuration for the FFT
static arm_rfft_fast_instance_f32 fft_instance;

// Arrays for FFT input and output
static float32_t fft_input[FFT_SIZE];
static float32_t fft_output[FFT_SIZE];

// Initialize the FFT
int frequency_init(void)
{
    arm_status status;

    status = arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);

    if (status == ARM_MATH_SUCCESS)
    {
        return 1;
    }

    return 0;
}

// Find the strongest frequency in microphone samples
uint32_t frequency_detect(volatile uint16_t *samples)
{
    float32_t average = 0.0f;

    // Calculate average ADC value
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        average += samples[i];
    }

    average /= FFT_SIZE;

    // Remove the microphone's DC offset
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        fft_input[i] = samples[i] - average;
    }

    // Perform the FFT
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);

    float32_t highest_power = 0.0f;
    uint32_t strongest_bin = 0;

    // Search frequencies from 62.5 Hz to 1000 Hz
    for (uint32_t bin = 4; bin <= 64; bin++)
    {
        float32_t real = fft_output[2 * bin];
        float32_t imag = fft_output[2 * bin + 1];

        // Magnitude squared (avoids calculating square roots)
        float32_t power = real * real + imag * imag;

        if (power > highest_power)
        {
            highest_power = power;
            strongest_bin = bin;
        }
    }

    // Convert FFT bin number into frequency in Hz
    uint32_t frequency = strongest_bin * SAMPLE_RATE / FFT_SIZE;

    return frequency;
}
