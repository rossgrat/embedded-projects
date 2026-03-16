#include <stdint.h>

#define RCC_BASE            0x40021000
#define GPIOB_BASE          0x48000400
#define TIM6_BASE           0x40001000

// RCC_AHB2ENR = Reset Clock Control, Advance High-Speed Bus 2, Enable Register
#define RCC_AHB2ENR_OFFSET  0x4C
#define RCC_AHB2ENR         (RCC_BASE + RCC_AHB2ENR_OFFSET)

#define GPIOB_MODER_OFFSET  0x00
#define GPIOB_MODER         (GPIOB_BASE + GPIOB_MODER_OFFSET)

#define GPIOB_ODR_OFFSET    0x14
#define GPIOB_ODR           (GPIOB_BASE + GPIOB_ODR_OFFSET)

#define RCC_APB1ENR1_OFFSET 0x58
#define RCC_APB1ENR1        (RCC_BASE + RCC_APB1ENR1_OFFSET)

#define TIMX_CR1            0x00                                // Timer Control Register
#define TIMX_DIER           0x0C                                // DMA/Interrupt Register
#define TIMX_SR             0x10                                // Status Register
#define TIMX_CNT            0x24                                // Counter
#define TIMX_PSC            0x28                                // Prescaler
#define TIMX_ARR            0x2C                                // Auto-Reload Register

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

    // Enable Timer 6 (Basic Timer)
    volatile uint32_t* rcc_apb1enr1 = (volatile uint32_t*)RCC_APB1ENR1;
    *rcc_apb1enr1 = *rcc_apb1enr1 | (1 << 4);


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
