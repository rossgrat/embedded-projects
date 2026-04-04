#include <stdint.h>


typedef union {
    volatile uint32_t* address;
    void(*handler)(void);
} Entry;

void main(void);
void toggleLight(void);

extern uint32_t _estack;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sidata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler() {
    // Copy .data from flash to RAM on-chip
    volatile uint32_t* flashData = &_sidata;
    volatile uint32_t* ramData = &_sdata;
    while (ramData != &_edata) {
        *ramData = *flashData;
        ramData++;
        flashData++;
    }

    // Zero out .bss
    volatile uint32_t* ramBss = &_sbss;
    while (ramBss != &_ebss) {
        *ramBss = 0;
        ramBss++;
    }

    main();
}

void Hard_Fault() {
    for(;;){}
}


//
// The  [0] = ... syntax is C syntax for a designated array.
// Each vector table entry is 4 bytes, and must be in the correct address
// spot for the designated interrupt. Here we are only setting some
// array entries, the rest are initialzed to 0, which the vector table
// understands to be a no-op.

__attribute__((section(".isr_vector")))
Entry table[71] = {
    [0] = { .address = (volatile uint32_t*)&_estack},
    [1] = { .handler = Reset_Handler},
    [2] = { .handler = Hard_Fault},
    [70] = {.handler = toggleLight}     // TIM6 Interrupt Address: 0x0000 0118 / 0x118 = 280 / 4 = 70
};
