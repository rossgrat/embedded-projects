# Register-level LED exercises

Drive the Nucleo-L4R5ZI's blue LED on PB7 with C startup code, a linker script, and direct register access.
The [blink source](c/blink/src/main.c) uses TIM6 interrupts; [pulse](c/pulse) is a separate exercise.

## Build

Use `arm-none-eabi-gcc` and `probe-rs`, as named in the exercise [Makefile](c/blink/Makefile).
From `c/blink`, build the binary explicitly:

```sh
mkdir -p bin
make bin/blink.bin
```

With the matching board connected, `make flash` writes and verifies the binary at `0x08000000`, then resets the chip.
The default Make target only builds an object, so it is not the full binary-build command.

## Concepts and completion

The startup vector table supplies the initial stack pointer and reset handler.
Startup copies `.data`, clears `.bss`, and calls `main`.
The [linker script](c/blink/src/link.ld) declares flash/RAM and exports the startup symbols.

GPIO setup enables the GPIOB clock and selects PB7 output mode before timer-driven toggling.
The source records `RCC_AHB2ENR` at `0x4002104C`, `GPIOB_MODER` at `0x48000400`, and `GPIOB_ODR` at `0x48000414`.
Complete the exercise by explaining the reset-to-interrupt path and observing the expected LED timing on hardware.

Use the [STM32 reference manual](https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) and [Cortex-M4 programming manual](https://www.st.com/resource/en/programming_manual/dm00046982-stm32-cortex-m4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf) for register definitions.
