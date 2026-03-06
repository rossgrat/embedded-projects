# Das Blinkenlights — Bare Metal LED Blink on STM32L4R5ZI

## Goal

Blink an LED on the Nucleo-L4R5ZI by writing directly to hardware registers.
No HAL, no CubeMX, no IDE — just gcc, a linker script, and st-flash.

## Hardware

- **Board:** ST Nucleo-L4R5ZI
- **MCU:** STM32L4R5ZIT6 (Cortex-M4, 2MB flash, 640KB SRAM)
- **LED:** LD2 (blue) on PB7
- **Clock:** 4 MHz MSI internal oscillator (default after reset, no PLL config needed)
- **Connection:** Micro-USB to USB-C cable to MacBook Air

## Toolchain

- `arm-none-eabi-gcc` — cross-compiler (install via `brew install --cask gcc-arm-embedded`)
- `st-flash` — flasher (install via `brew install stlink`)
  - [repo](https://github.com/stlink-org/stlink)

## Files to Write

### 1. `main.c` — Application logic
- Define register addresses via `volatile uint32_t *` pointers:
  - `RCC_AHB2ENR` (0x4002104C) — enable GPIOB clock (bit 1)
  - `GPIOB_MODER` (0x48000400) — set PB7 to output (bits [15:14] = 01)
  - `GPIOB_ODR` (0x48000414) — toggle PB7 (bit 7)
- Busy-wait delay loop (~200000 iterations for visible blink at 4 MHz)
- Infinite loop: toggle, delay, repeat

### 2. `startup.c` — Vector table and reset handler
- Vector table (array in `.isr_vector` section):
  - Entry 0: initial stack pointer (`_estack`, top of RAM)
  - Entry 1: `Reset_Handler` address
- `Reset_Handler`:
  - Copy `.data` from flash to RAM
  - Zero `.bss`
  - Call `main()`

### 3. `link.ld` — Linker script
- Memory regions:
  - FLASH: origin 0x08000000, length 2048K
  - RAM: origin 0x20000000, length 640K
- Sections:
  - `.isr_vector` → FLASH (first, so vector table is at 0x08000000)
  - `.text` → FLASH
  - `.data` → RAM (load address in FLASH, copied by startup code)
  - `.bss` → RAM (zeroed by startup code)
- Export symbols: `_estack`, `_sdata`, `_edata`, `_sidata`, `_sbss`, `_ebss`

### 4. `Makefile` — Build and flash
- Compile: `arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -Os`
- Link: `-T link.ld -nostdlib`
- Convert: `arm-none-eabi-objcopy -O binary blink.elf blink.bin`
- Flash: `st-flash write blink.bin 0x08000000`

## Build & Flash

```
make          # produces blink.bin
make flash    # writes to board via ST-Link
```

## What happens at runtime

1. Chip resets, reads vector table at 0x08000000
2. Loads stack pointer from entry 0
3. Jumps to Reset_Handler from entry 1
4. Reset_Handler copies .data, zeros .bss, calls main()
5. main() enables GPIOB clock, sets PB7 to output, toggles in a loop
6. Blue LED blinks
