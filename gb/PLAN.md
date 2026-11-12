# Game Boy emulator plan

Build a bare-metal DMG emulator for Pokémon Red on the Nucleo-L4R5ZI.
Use the ST-Link UART connection for terminal input and a text display.
The proposed platform uses a 120 MHz clock, 2 MiB flash, and 640 KiB SRAM.
Keep the emulator and ROM inside the flash budget; reserve SRAM for machine state, framebuffers, and serial buffers.

## Checkpoints

1. Bring up clock and UART, embed the ROM, and check its title and header checksum.
2. Implement the SM83 CPU and cycle counts; pass Blargg's `cpu_instrs` tests.
3. Implement the memory bus and cartridge banking; pass `mem_timing` and check bank reads.
4. Implement timers and interrupts; pass instruction/timer timing tests.
5. Implement scanline graphics and LCD timing; render `dmg-acid2` and the game's intro correctly.
6. Render 160×144 pixels as a 160×72 half-block terminal display, using changed-cell updates to limit UART traffic.
7. Map raw terminal keys to joypad buttons and reach the game from its title screen.
8. Add pacing and save persistence; start a game, leave the room, save, and reload.

Confirm the cartridge's exact mapper, ROM size, and save-RAM requirements from its header before choosing the memory implementation.
The earlier plan proposed MBC3 and a roughly 1 MiB ROM.
Audio, Game Boy Color, and link-cable support remain outside the first version.

## Development

Use `arm-none-eabi-gcc`, `probe-rs`, a raw-key serial terminal, and a ROM-to-C-array step.
Develop startup/linking, CPU, memory, graphics, timers, input, display, and UART as separate units.
Test CPU and graphics logic on the host when useful, and use SWD/GDB for board-specific work.
CPU accuracy, LCD interrupt timing, and UART bandwidth are the main checkpoints before playability.

References: [Pan Docs](https://gbdev.io/pandocs/), [test ROMs](https://github.com/c-sp/gameboy-test-roms), and the [STM32 manual](https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf).
