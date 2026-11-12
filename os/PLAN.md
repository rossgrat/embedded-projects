# RTOS and shell plan

Build a preemptive kernel and UART shell on the Nucleo-L4R5ZI without HAL, CubeMX, or libc.
Use register-level C, assembly context switching, a linker script, and `probe-rs`.
The proposed clock is 120 MHz; the board's ST-Link connection carries the serial console.

## Checkpoints

1. Bring up startup, memory layout, and the clock. Demonstrate a known LED rate.
2. Add interrupt-driven UART with receive/transmit buffers and minimal character/formatted I/O. Demonstrate echo.
3. Add line editing, tokenization, and command dispatch. Demonstrate `help`, `echo`, and `clear`.
4. Add task stacks and control blocks, context switching, and a scheduler. Run two LED tasks and the shell concurrently.
5. Add sleep, mutexes, semaphores, and a `WFI` idle task. Demonstrate task listing and producer/consumer synchronization.
6. Add a RAM filesystem. Create directories, write files, and read them within one session.
7. Add the remaining task, memory, and filesystem commands. Demonstrate a complete shell session.

Start with round-robin scheduling, then add fixed-priority preemption.
The task record holds its stack, saved registers, state, and priority.
Use PSP for tasks and MSP for handlers.
The PendSV switch preserves r4-r11 and PSP; exception entry handles the remaining hardware-stacked registers.
The first task needs an initial exception-return frame.

## Constraints and tests

SysTick triggers rescheduling through `SCB_ICSR.PENDSVSET`.
Keep PendSV at the lowest interrupt priority.
Protect shared UART/formatted-output access before multiple tasks use it.
Inspect task stacks and PSP/MSP with SWD/GDB, then stress many tasks, short sleeps, and shared UART traffic.
Complete each standalone demonstration before integration.

Reserve SRAM for globals, task stacks, the RAM filesystem, and the handler stack.
Persistent storage is outside the first version.
Task commands include `ps`, `sleep`, `uptime`, `kill`, and `top`; filesystem commands include `ls`, `cat`, `mkdir`, `rm`, `touch`, `pwd`, and `cd`.

References: [STM32 manual](https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) and [Cortex-M4 programming manual](https://www.st.com/resource/en/programming_manual/dm00046982-stm32-cortex-m4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf).
