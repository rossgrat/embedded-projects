#include <stdint.h>

#define RCC_BASE            0x40021000
#define GPIOB_BASE          0x48000400

// RCC_AHB2ENR = Reset Clock Control, Advance High-Speed Bus 2, Enable Register
#define RCC_AHB2ENR_OFFSET  0x4C
#define RCC_AHB2ENR         (RCC_BASE + RCC_AHB2ENR_OFFSET)

#define GPIOB_MODER_OFFSET  0x00
#define GPIOB_MODER         (GPIOB_BASE + GPIOB_MODER_OFFSET)

#define GPIOB_ODR_OFFSET    0x14
#define GPIOB_ODR           (GPIOB_BASE + GPIOB_ODR_OFFSET)

void main() {

    // Enable Port B (Pins GPIOB1, GPIOB2, etc.)
    volatile uint32_t* rcc_ahb2enr = (volatile uint32_t*)RCC_AHB2ENR;
    *rcc_ahb2enr = *rcc_ahb2enr | (1 << 1);

    // Set Port B mode to output (01 = General Purpose Output)
    volatile uint32_t* gpiob_moder = (volatile uint32_t*)GPIOB_MODER;
    // Reset pin values
    *gpiob_moder = *gpiob_moder & ~(3 << 14);
    // Set output mode
    *gpiob_moder = *gpiob_moder | (1 << 14);


    volatile uint32_t* gpiob_odr = (volatile uint32_t*)GPIOB_ODR;
    // Reset
    *gpiob_odr = *gpiob_odr & ~(1 << 7);

    volatile int counter = 0;
    for (;;) {
        if (counter == 10000) {
            // Flip light
            *gpiob_odr = *gpiob_odr ^ (1 << 7);
            counter = 0;
        } else {
            counter++;
        }
    }

}
