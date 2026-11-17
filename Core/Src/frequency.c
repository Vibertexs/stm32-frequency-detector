
#include "arm_math.h"
#include <stdint.h>
#include <math.h>

#define FFT_SIZE 1024
#define SAMPLE_RATE 16000

// FFT configuration
static arm_rfft_fast_instance_f32 fft_instance;

// FFT input and output arrays
static float32_t fft_input[FFT_SIZE];
static float32_t fft_output[FFT_SIZE];

// Hann window coefficients
static float32_t hann_window[FFT_SIZE];

// Initialize the FFT and Hann window
int frequency_init(void)
{
    arm_status status;

    // Initialize the 1024-point FFT
    status = arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);

    if (status != ARM_MATH_SUCCESS)
    {
        return 0;
    }

    // Calculate the Hann window once
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        float32_t angle = (2.0f * PI * i) / (FFT_SIZE - 1);

        hann_window[i] = 0.5f - 0.5f * cosf(angle);
    }

    return 1;
}

// Calculate power at a particular FFT bin
static float32_t frequency_bin_power(uint32_t bin)
{
    float32_t real = fft_output[2 * bin];
    float32_t imag = fft_output[2 * bin + 1];

    return real * real + imag * imag;
}

// Detect frequency in tenths of a Hz
// Example: return 4402 means 440.2 Hz
uint32_t frequency_detect(volatile uint16_t *samples)
{
    float32_t average = 0.0f;

    // Calculate average ADC value
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        average += samples[i];
    }

    average /= FFT_SIZE;

    // Remove DC offset and apply Hann window
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        fft_input[i] = (samples[i] - average) * hann_window[i];
    }

    // Perform the FFT
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);

    float32_t highest_power = 0.0f;
    uint32_t strongest_bin = 0;

    // Find strongest bin from 62.5 Hz to 1000 Hz
    for (uint32_t bin = 4; bin <= 64; bin++)
    {
        float32_t power = frequency_bin_power(bin);

        if (power > highest_power)
        {
            highest_power = power;
            strongest_bin = bin;
        }
    }

    // No detectable FFT power
    if (highest_power <= 0.0f)
    {
        return 0;
    }

    // Get power of the strongest bin and its neighbors
    float32_t left_power = frequency_bin_power(strongest_bin - 1);
    float32_t center_power = frequency_bin_power(strongest_bin);
    float32_t right_power = frequency_bin_power(strongest_bin + 1);

    // Use logarithmic power for quadratic interpolation
    float32_t left = logf(left_power + 0.000001f);
    float32_t center = logf(center_power + 0.000001f);
    float32_t right = logf(right_power + 0.000001f);

    // Estimate how far the peak is from the center bin
    float32_t denominator = left - 2.0f * center + right;
    float32_t offset = 0.0f;

    if (fabsf(denominator) > 0.000001f)
    {
        offset = 0.5f * (left - right) / denominator;
    }

    // Limit interpolation to half a bin
    if (offset > 0.5f)
    {
        offset = 0.5f;
    }

    if (offset < -0.5f)
    {
        offset = -0.5f;
    }

    // Combine the strongest bin with the fractional offset
    float32_t interpolated_bin = strongest_bin + offset;

    // Convert the interpolated bin to frequency in Hz
    float32_t frequency = interpolated_bin * SAMPLE_RATE / FFT_SIZE;

    // Return tenths of a Hz, rounded
    return (uint32_t)(frequency * 10.0f + 0.5f);
}
