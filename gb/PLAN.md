# Game Boy Emulator on STM32L4R5ZI — Pokémon Red

[Reference Manual](https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
[Pan Docs (GB hardware reference)](https://gbdev.io/pandocs/)
[Blargg + Mooneye test ROMs](https://github.com/c-sp/gameboy-test-roms)

## Goal

A bare-metal original Game Boy (DMG) emulator that runs **Pokémon Red** on the
Nucleo. The game is controlled from a macOS terminal (keyboard → UART → buttons)
and the screen is rendered back into that same terminal as text. No HAL, no
libc — our own runtime, just like the other projects here.

## Why Pokémon Red (not Crystal)

- **DMG, not Game Boy Color.** Monochrome 4-shade PPU — dramatically simpler
  than CGB color/palette/banking.
- **~1 MB ROM** (1,048,576 bytes) — fits in the **2 MB internal flash** as a
  `const` array alongside the emulator. No external QSPI/SD storage needed.
- **MBC3 mapper, battery SRAM for saves**, no RTC dependency for core play.
  *(Verify the exact mapper/SRAM size against the cart header before relying on it.)*

This keeps the whole thing self-contained on the board.

## Hardware

- **Board:** ST Nucleo-L4R5ZI
- **MCU:** STM32L4R5ZIT6 (Cortex-M4F, 2MB flash, 640KB SRAM)
- **Clock:** PLL to 120 MHz (plenty for the GB's ~4.19 MHz CPU)
- **I/O:** USART over ST-Link USB virtual COM port — both keyboard input and
  screen output ride this one serial link

The GB has a 64KB address space and ~64KB of internal RAM total; against 640KB
SRAM, compute and memory are comfortable. The two real constraints are
**flash budget** (code + 1MB ROM under 2MB) and **UART bandwidth** (the framerate
limiter — see Display).

## Toolchain

- `arm-none-eabi-gcc` — cross-compiler (`brew install --cask gcc-arm-embedded`)
- `probe-rs` — flash + debug
- A serial terminal that passes raw keys (`screen`, `tio`, or a small host script)
- A tool to convert `pokemon_red.gb` → a C array (`xxd -i` or a Python script)

## The Game Boy, briefly

- **CPU:** Sharp LR35902 ("SM83"), ~4.194304 MHz. Z80-like but its own ISA.
  ~500 opcodes incl. the `CB`-prefixed bank.
- **Display:** 160×144 px, 4 shades of gray. Tile-based background + window +
  sprites (OAM).
- **Timing:** everything is measured in CPU cycles; the PPU runs in lockstep
  with the CPU across scanline modes (OAM scan → drawing → H-blank → V-blank).

---

## Milestone Ladder

Each milestone has a hard, externally-verifiable checkpoint. Emulators succeed
or fail on accuracy, so lean on the published test ROMs.

### M0 — Bare-metal bring-up + ROM in flash
Clock to 120 MHz, UART console. Bake `pokemon_red.gb` into a flash section as a
`const uint8_t[]`.
*Checkpoint:* dump the cart header over UART — title reads `POKEMON RED`, and the
header checksum validates.

### M1 — CPU core
Implement the SM83: registers, flags, the full opcode + `CB` tables, interrupt
dispatch (IME, IE/IF), accurate per-instruction cycle counts.
*Checkpoint:* **pass Blargg's `cpu_instrs`** test ROM. Non-negotiable — a CPU
that's 99% right will fail Pokémon in subtle, un-debuggable ways.

### M2 — Memory map + MBC3
The full bus: ROM banks, VRAM, WRAM, OAM, I/O registers, HRAM. MBC3 bank
switching for ROM, and battery-backed cart SRAM (back it with internal flash or
a RAM buffer for v1).
*Checkpoint:* `mem_timing` test ROM passes; bank switches read correct data.

### M3 — Timers + interrupts
DIV/TIMA/TMA/TAC timer, the interrupt sources (V-blank, LCD STAT, timer, serial,
joypad).
*Checkpoint:* `instr_timing` / timer test ROMs pass.

### M4 — PPU (graphics)
Scanline renderer into a 160×144 framebuffer: background, window, sprites, the
LCDC/STAT/scroll/palette registers, and the mode timing that drives STAT
interrupts.
*Checkpoint:* `dmg-acid2` renders correctly; Pokémon Red's intro (Game Freak
logo → Nidorino/Gengar → title) draws.

### M5 — Display to terminal
Push the framebuffer out the UART as text. **Half-block + ANSI grayscale** is the
sweet spot: each `▀` cell encodes two vertical pixels via fg/bg color, giving a
160×72-character image at full GB resolution.
- Budget: a naive full redraw is ~10KB+ of escape codes per frame → UART is the
  bottleneck. Mitigate with **delta encoding** (only redraw changed cells) and a
  sane baud rate (try 115200, push higher if the VCP allows).
*Checkpoint:* the title screen is recognizable in the terminal at a usable rate.

### M6 — Input
macOS keypress → terminal raw mode → byte over UART → mapped to the GB joypad
register (D-pad + A/B/Start/Select), wired to the joypad interrupt.
*Checkpoint:* press Start at the title screen and advance into the game.

### M7 — Playability
Frame pacing to feel right, SRAM save persistence (write cart SRAM back to a
flash sector), input latency tuning.
*Checkpoint:* start a new game, walk out of the player's room, save and reload.

**Out of scope for v1:** audio (the APU is hard and the terminal can't play it
anyway), Game Boy Color features, link cable.

---

## Memory Layout

```
0x08000000  +-----------------------+
            | .isr_vector / .text   |  emulator code
0x08020000  | ROM image (~1MB)      |  pokemon_red.gb as const array
            | SRAM save sector(s)   |  battery RAM persisted here (M7)
0x08200000  +-----------------------+  (2MB flash ceiling)

0x20000000  +-----------------------+
            | GB WRAM / VRAM / OAM   |  emulated machine state (~64KB)
            | framebuffer 160x144    |  + previous-frame buffer for deltas
            | UART TX ring buffer    |  large — display output is bursty
0x200A0000  +-----------------------+  (640KB SRAM ceiling)
```

## Files to Write

- `startup.c`, `link.ld` — runtime + memory regions (ROM section defined here)
- `cpu.c` — SM83 core, opcode tables, interrupt dispatch
- `mmu.c` — bus + MBC3 mapper
- `ppu.c` — scanline renderer + LCD timing
- `timer.c` — DIV/TIMA + interrupt sources
- `joypad.c` — input register
- `display.c` — framebuffer → ANSI/half-block, delta encoder
- `uart.c` — interrupt-driven console (shared in/out)
- `rom.c` (generated) — the `pokemon_red.gb` byte array
- `Makefile` — build, ROM-to-array conversion, `probe-rs` flash

## Hardest Parts (de-risk these)

1. **CPU accuracy.** Pass the test ROMs *before* trusting anything downstream.
2. **PPU timing.** STAT-mode timing drives mid-frame effects; get it wrong and
   graphics glitch in game-specific ways.
3. **UART as the display bus.** Without delta encoding the framerate is
   unplayable. This is the make-or-break of the "terminal screen" concept.

## Testing Strategy

- Drive development with the Blargg/Mooneye/acid2 test ROMs at each milestone.
- Optionally build the same `cpu.c`/`ppu.c` as a host (Mac) binary first, with
  an SDL or terminal frontend, to debug logic with full tooling before flashing.
- SWD + GDB on-board for the hardware-specific bring-up.
