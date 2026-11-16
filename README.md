# STM32 Audio Frequency Detector

Real-time audio frequency detector for the STM32L476RG (Nucleo-L476RG). It samples a microphone with the ADC, runs a 1024-point FFT using CMSIS-DSP, and reports the strongest frequency over UART.

## How it works

1. **TIM6** triggers the ADC at 16 kHz.
2. **DMA** moves 1024 samples into a buffer without CPU involvement.
3. Once the buffer is full, a **1024-point real FFT** (`arm_rfft_fast_f32`) is computed.
4. The strongest bin is converted to Hz: `frequency = bin * 16000 / 1024` (15.6 Hz resolution).
5. The result is printed over UART along with min, max, average and peak-to-peak of the raw samples, which is useful for checking the mic signal level.

## Hardware

- STM32 Nucleo-L476RG
- Analog microphone module connected to the ADC input (TODO: add pin, e.g. `PA0`)
- USB cable (ST-LINK virtual COM port for output)

## Output

Open the ST-LINK virtual COM port in Tera Term (or any serial terminal) at **115200 baud, 8N1**:

```
FFT initialized successfully!
Frequency: 440 Hz
min=1850 max=2250 avg=2048 pp=400
```

## Project layout

| Path | Contents |
|------|----------|
| `Core/Src/main.c` | Main loop and clock/GPIO setup |
| `Core/Src/microphone.c` | ADC + DMA + TIM6 capture |
| `Core/Src/frequency.c` | FFT and peak detection |
| `Core/Src/debug.c` | UART printing helpers |
| `DSP/` | CMSIS-DSP sources used for the FFT |
| `Drivers/` | STM32 HAL and CMSIS device drivers |
| `Final Project.ioc` | STM32CubeMX configuration |

## Building

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).
2. `File > Import > Existing Projects into Workspace` and select this folder.
3. Build, then run/debug on the Nucleo board.

## Limitations

- Detects a single dominant frequency, so it works best with a pure tone.
- Maximum detectable frequency is 8 kHz (Nyquist for a 16 kHz sample rate).
