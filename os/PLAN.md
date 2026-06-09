# A Custom RTOS for STM32L4R5ZI

[Reference Manual](https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
[ARM Cortex-M4 Programming Manual](https://www.st.com/resource/en/programming_manual/dm00046982-stm32-cortex-m4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)
[ARMv7-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0403/latest/)

## Goal

A preemptive multitasking RTOS on bare metal, with an interactive shell
exposing Linux-style commands over the UART console. No HAL, no CubeMX, no
libc — just gcc, a linker script, our own runtime, and probe-rs.

The two pillars: a **scheduler** that switches between independent tasks, and a
**shell** that feels like a tiny Unix terminal.

## Hardware

- **Board:** ST Nucleo-L4R5ZI
- **MCU:** STM32L4R5ZIT6 (Cortex-M4F, 2MB flash, 640KB SRAM)
- **Clock:** PLL to 120 MHz (the scheduler tick and UART baud both derive from this)
- **Console:** USART over the on-board ST-Link USB virtual COM port (no extra cable)

## Toolchain

- `arm-none-eabi-gcc` — cross-compiler (`brew install --cask gcc-arm-embedded`)
- `probe-rs` — flash + debug (matches the das-blinkenlights Makefile)
- A serial terminal on the Mac (`screen /dev/cu.usbmodem* 115200`, minicom, or `tio`)

## Design Decisions

- **From scratch.** Own startup, linker script, register-level drivers. No HAL,
  no newlib. We provide the handful of primitives we need ourselves.
- **Full RTOS.** Preemptive, not cooperative. The whole point is real context
  switching and the concurrency bugs that come with it.

---

## Milestone Ladder

Each milestone is independently demoable. Don't start the next until the
current one is rock-solid — the scheduler is unforgiving of shaky foundations.

### M0 — Bare-metal bring-up
Reuse the das-blinkenlights foundation. Clock tree to 120 MHz via PLL.
Vector table, Reset_Handler (.data copy, .bss zero), linker script.
*Demo:* LED blinks at a known rate, proving the clock is right.

### M1 — UART console
Interrupt-driven USART RX/TX with ring buffers. A `putchar`/`getchar` layer,
then a minimal `printf` (our own — no libc).
*Demo:* type characters, see them echo back over the terminal.

### M2 — The shell
Line editor (backspace, simple history), tokenizer, and a command dispatch
table (`{name, fn, help}`). Built-ins first: `help`, `echo`, `clear`.
*Demo:* an interactive prompt that runs commands.

### M3 — The scheduler (the RTOS leap)
This is the hard part. Build it in order:
- **Task control block** — per-task stack, saved register context, state
  (READY/RUNNING/BLOCKED), priority.
- **Context switch** — `PendSV` handler in assembly: stash r4-r11 + PSP of the
  outgoing task, restore the incoming one. Hardware stacks r0-r3/r12/lr/pc/xpsr
  automatically. Use the **PSP** for tasks, **MSP** for handlers.
- **Tick** — `SysTick` at e.g. 1 kHz drives time-slicing; trigger PendSV to
  reschedule.
- **Scheduler policy** — start round-robin, then add fixed-priority preemption.
- **First task launch** — the fiddly bootstrap: hand-craft a fake stack frame
  so the first PendSV "returns" into your task.
*Demo:* two tasks blink two LEDs at different rates with no busy-waits; the
shell runs as its own task while a background task ticks.

### M4 — Sync primitives & sleep
`sleep(ms)` (yield + wake on tick), then a mutex and a counting semaphore
(needed the moment two tasks touch the UART). Idle task runs `WFI`.
*Demo:* `ps`-style command lists tasks/states; producer/consumer demo.

### M5 — Filesystem
A RAM-disk: in-memory tree of inodes + a fixed data arena. Enough to back the
file commands. (Persistent FS on external flash is explicitly out of scope for
v1 — keep the ROM/flash budget for code.)
*Demo:* `mkdir`, `touch`, write and `cat` a file, survive within a session.

### M6 — Linux-style command set
Layer commands onto the primitives built above:

| Needs nothing | Needs scheduler | Needs filesystem |
|---|---|---|
| `uname` `echo` `clear` `reboot` | `ps` `sleep` `uptime` `kill` | `ls` `cat` `mkdir` `rm` `touch` `pwd` `cd` |
| `free` (SRAM stats) `peek`/`poke` | `top` (live task view) | `echo > file` (redirect) |

*Demo:* a session that feels like a stripped-down Unix shell.

---

## Memory Layout

```
0x08000000  +-----------------------+
            | .isr_vector / .text   |  vector table, kernel + shell code
            | .rodata               |  command tables, help strings
0x08200000  +-----------------------+  (2MB flash ceiling)

0x20000000  +-----------------------+
            | .data / .bss          |  kernel globals
            | per-task stacks        |  one region per TCB
            | RAM-disk arena         |  filesystem storage
            | kernel/main stack (MSP)|  grows down from top (_estack)
0x200A0000  +-----------------------+  (640KB SRAM ceiling)
```

## Key Registers / Core Peripherals

- `SysTick` (SCB) — scheduler tick (`SYST_RVR`, `SYST_CSR`)
- `SCB_ICSR` — set `PENDSVSET` to trigger a context switch
- `SCB_SHPR` — interrupt priorities; **PendSV must be lowest** so it only
  preempts at a safe point
- `CONTROL` register — select PSP for tasks, MSP for the kernel
- `RCC_PLLCFGR` / `RCC_CR` — 120 MHz clock setup
- `USARTx_CR1/BRR/ISR/RDR/TDR` — console

## Files to Write

- `startup.c` — vector table, reset handler
- `link.ld` — memory regions + sections (per-task stacks reserved here)
- `sched.c` / `switch.S` — TCBs, scheduler, PendSV context switch
- `sync.c` — mutex, semaphore, sleep
- `uart.c` — interrupt-driven console
- `printf.c` — minimal formatted output
- `shell.c` — line editor, parser, dispatch table
- `commands/*.c` — one file (or one table) of command implementations
- `ramfs.c` — in-memory filesystem
- `Makefile` — build + `probe-rs` flash

## Hardest Parts (de-risk these)

1. **The first context switch** — bootstrapping a fake stack frame and the
   PendSV exception-return dance. Expect to live in GDB here.
2. **Priority of PendSV vs. SysTick** — get this wrong and you corrupt task
   stacks intermittently.
3. **Reentrancy** — the moment the shell task and a background task both call
   `printf`, you need that mutex.

## Testing Strategy

- SWD + GDB throughout; inspect PSP/MSP and TCB stacks across a switch.
- Each milestone has a standalone demo (above) before integration.
- Stress the scheduler: many tasks, tight sleeps, hammer the shared UART.
