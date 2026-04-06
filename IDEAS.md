# Project Ideas

## Foundational

- **UART serial output** — send text to a computer over serial. Enables printf-style debugging.
- **SPI / I2C** — communicate with an external sensor (temperature, accelerometer, etc.).
- **ADC** — read analog input (potentiometer, light sensor). Combine with PWM for physical brightness control.
- **DAC** — generate waveforms (sine, sawtooth) and output to a speaker. TIM6 can trigger the DAC directly.
- **DMA** — transfer data between peripherals and memory without the CPU (e.g., continuous ADC sampling into a buffer).
- **Sleep modes** — replace `for(;;){}` with WFI. Explore STM32L4 low-power modes.
- **Clock configuration** — configure the PLL to run at 80MHz+. Learn the full clock tree (MSI -> PLL -> SYSCLK -> AHB/APB prescalers).

## Intermediate

- **Command shell** — accept text commands over UART to control the board. Teaches input parsing and interrupt-driven buffering.
- **Bootloader** — receive new firmware over UART and write it to flash. Board updates itself without a debugger.
- **Data logger** — read sensors periodically, store to external SPI flash or SD card, dump over UART.
- **Display driver** — drive an OLED or LCD over SPI/I2C. Render text, sensor readings, or simple graphics.
- **Digital oscilloscope** — sample analog signals with ADC + DMA, send waveform data to a computer over UART.
- **Audio player** — DMA from memory to DAC, triggered by a timer, to play audio samples. Add an SD card for WAV playback.
- **LED strip control** — drive WS2812/NeoPixel strips using timer PWM + DMA for precisely timed signals without CPU involvement.

## Advanced

- **Battery-powered sensor node** — wake from deep sleep, read a sensor, transmit data, go back to sleep. Optimize for lowest power consumption.
- **RTOS** — port FreeRTOS. Run multiple concurrent tasks (sensors, display, UART) with proper scheduling.
- **Motor control** — use TIM1 advanced timer with complementary PWM, dead-time insertion, and ADC feedback to drive a brushless motor.
- **Custom USB device** — use the STM32L4 USB peripheral to appear as a HID device (keyboard, gamepad) or virtual COM port.
