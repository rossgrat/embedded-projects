# Application-Space Bootloader for STM32L4R5ZI

## Concept

Write a custom bootloader that lives in the first sector of flash and can
receive new application firmware over UART, write it to flash, and jump to it.
This is how commercial products do field firmware updates without a debug probe.

This is NOT replacing ST's factory bootloader (that lives in protected ROM at
0x1FFF0000 and can never be erased). This bootloader lives in normal flash
alongside the application.

## Memory Layout

```
0x08000000  +-----------------------+
            | Bootloader (16KB)     |  ← chip boots here, our bootloader runs
0x08004000  +-----------------------+
            | Application (rest)    |  ← bootloader loads/jumps to here
            +-----------------------+

0x1FFF0000  +-----------------------+
            | ST factory bootloader |  ← permanent ROM, not involved
            +-----------------------+
```

The bootloader is linked with FLASH origin at 0x08000000 (normal).
The application is linked with FLASH origin at 0x08004000 (offset).

## Boot Flow

1. Chip resets → executes bootloader at 0x08000000
2. Bootloader checks a condition (button held? magic byte in RAM? UART activity?)
3. **If update mode:**
   - Initialize UART
   - Receive application binary over serial (define a simple protocol)
   - Unlock flash controller (write unlock sequence to FLASH_KEYR)
   - Erase application sectors
   - Write received bytes to flash starting at 0x08004000
   - Lock flash
   - Reset or jump to application
4. **If normal mode:**
   - Jump directly to application at 0x08004000

## Jumping to the Application

The application has its own vector table at 0x08004000. To jump to it:

1. Read word at 0x08004000 → application's initial stack pointer
2. Read word at 0x08004004 → application's Reset_Handler address
3. Set the main stack pointer (MSP) to the value from step 1
4. Relocate the vector table (write 0x08004000 to SCB->VTOR)
5. Jump to the address from step 2

## Prerequisites / Building Blocks

These should be understood and tested independently before combining:

1. **Bare-metal register programming** — the das-blinkenlights project
2. **UART communication** — configure USART registers, send/receive bytes
   between the Nucleo and a host machine (minicom or a Python script)
3. **Flash controller** — understand the STM32L4 flash programming sequence:
   unlock, erase, program (double-word writes on L4), lock
4. **A serial protocol** — define how the host sends the binary (start marker,
   size, data chunks, checksums, end marker)
5. **A host-side tool** — Python script or similar to send the binary over
   serial using the protocol above

## Key Registers

- `FLASH_KEYR` — unlock flash with magic sequence (0x45670123, then 0xCDEF89AB)
- `FLASH_SR` — status register (check BSY bit, error flags)
- `FLASH_CR` — control register (page erase, programming enable)
- `USARTx_BRR` — baud rate
- `USARTx_CR1` — enable USART, TX, RX
- `USARTx_ISR` — status flags (RXNE, TXE)
- `USARTx_RDR` / `USARTx_TDR` — receive/transmit data
- `SCB_VTOR` (0xE000ED08) — vector table offset register

## Testing Strategy

1. Flash the bootloader to 0x08000000 via st-flash (using SWD, as normal)
2. Build a test application (e.g. blink, linked to 0x08004000)
3. Send the application binary over UART from the Mac
4. Bootloader writes it and jumps to it
5. LED blinks = success

Can also use SWD + GDB to debug the bootloader while it's running.
