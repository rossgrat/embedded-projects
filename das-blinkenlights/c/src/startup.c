#include <stdint.h>


typedef union {
    volatile uint32_t* address;
    void(*handler)(void);
} Entry;

void main(void);

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


__attribute__((section(".isr_vector")))
Entry table[] = {
    { .address = (volatile uint32_t*)&_estack},
    { .handler = Reset_Handler},
};
