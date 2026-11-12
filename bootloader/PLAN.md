# UART bootloader plan

Build an application-space bootloader for STM32L4R5ZI that receives firmware over UART, writes application flash, and transfers control.
ST's factory ROM bootloader remains separate.

## Proposed layout and flow

Reserve 16 KiB from `0x08000000` for the bootloader and link the application at `0x08004000`.
Confirm the flash layout and erase/program rules against the target's [reference manual](https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) before implementation.

On reset, check the selected update trigger.
In update mode, initialize UART, receive the image, unlock flash, erase application pages, program the image, lock flash, and reset or transfer control.
Normal mode transfers control directly.

The application vector table provides its initial stack pointer at `0x08004000` and reset-handler address at `0x08004004`.
The handoff sets MSP, relocates `SCB->VTOR`, and branches to that reset handler.

## Prerequisites and checkpoints

1. Complete register-level LED work and independent UART send/receive.
2. Exercise flash unlock, erase, double-word programming, status/error checks, and locking.
3. Define a serial protocol with image size, chunks, checksums, and an end marker.
4. Build a host sender and an application linked at the proposed offset.
5. Flash the bootloader through the debug probe, upload the application through UART, and observe its LED behavior.

Use SWD and GDB to inspect the handoff.
Relevant registers are `FLASH_KEYR`, `FLASH_SR`, `FLASH_CR`, the USART baud/control/status/data registers, and `SCB_VTOR` at `0xE000ED08`.
The recorded flash unlock sequence is `0x45670123`, then `0xCDEF89AB`.
