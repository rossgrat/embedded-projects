#include <stdint.h>

#define RCC_BASE            0x40021000
#define GPIOB_BASE          0x48000400
// Timers are driven by SYSCLK. The default source for SYSCLK is MSI (Multi-speed Internal Oscillator)
// at 4MHz
#define TIM6_BASE           0x40001000

// RCC_AHB2ENR = Reset Clock Control, Advance High-Speed Bus 2, Enable Register
#define RCC_AHB2ENR_OFFSET  0x4C
#define RCC_AHB2ENR         (RCC_BASE + RCC_AHB2ENR_OFFSET)

#define GPIOX_ODR    0x14
#define GPIOX_MODER  0x00
#define GPIOX_AFRL   0x20

#define GPIOB_ODR           (GPIOB_BASE + GPIOX_ODR)
#define GPIOB_MODER         (GPIOB_BASE + GPIOX_MODER)
#define GPIOB_AFRL          (GPIOB_BASE + GPIOX_AFRL)

// RCC_APB1ENR1 = Reset Clock Control, Advanced Peripheral Bus 1, Enable Register 1
#define RCC_APB1ENR1_OFFSET 0x58
#define RCC_APB1ENR1        (RCC_BASE + RCC_APB1ENR1_OFFSET)

#define TIMX_CR1            0x00                                // Timer Control Register
#define TIMX_DIER           0x0C                                // DMA/Interrupt Register
#define TIMX_SR             0x10                                // Status Register
// #define TIMX_CNT            0x24                                // Counter
#define TIMX_PSC            0x28                                // Prescaler
#define TIMX_ARR            0x2C                                // Auto-Reload Register

#define TIM6_CR1            (TIM6_BASE + TIMX_CR1)
#define TIM6_SR             (TIM6_BASE + TIMX_SR)
#define TIM6_DIER           (TIM6_BASE + TIMX_DIER)
#define TIM6_PSC            (TIM6_BASE + TIMX_PSC)
#define TIM6_ARR            (TIM6_BASE + TIMX_ARR)


// ARM Core functionality, including the NVIC controller, is distinct from
// ST designed peripherals
#define NVIC_BASE 0xE000E000
#define NVIC_ISERX_OFFSET 0x100
#define NVIC_ISER1_OFFSET 0x04
#define NVIC_ISER1 (NVIC_BASE + NVIC_ISERX_OFFSET + NVIC_ISER1_OFFSET)

// The code below introduces a way of avoiding the verbose declaration
#define REG(addr) (*(volatile uint32_t*)(addr))

void toggleLight() {
    REG(GPIOB_ODR) ^= (1 << 7);

    // It is necessary to read the interrupt register after our write
    // and BEFORE returning from the interrupt handler, because the
    // write takes several cycles to propagate across the APB bus. The
    // hardware guarantees read-after-write ordering on the same peripheral,
    // so by waiting for a read, we guarantee that the write has propagated
    // to the register before returning from the interrupt handler.
    //
    // If we were to write the interrupt register, then immediately return
    // from the interrupt handler, the NVIC would see the UIF still set,
    // the interrupt request line still high, and would "tail chain" back into
    // the interrupt handler. This gives the appearance of the light never
    // blinking.
    REG(TIM6_SR) = 0; // Reset Interrupt Register
    (void)REG(TIM6_SR); // Read Interrupt Register
}

void main() {

    // Start configure blue light
    // Enable Port B (Pins GPIOB1, GPIOB2, etc.)
    REG(RCC_AHB2ENR) |= (1 << 1);

    // Set Port B mode to output (01 = Output)
    // Reset pin values
    REG(GPIOB_MODER) &= ~(3 << 14);
    // Set output mode
    REG(GPIOB_MODER) |= (1 << 14);
    // Reset
    REG(GPIOB_ODR) &= ~(1 << 7);


    // Start using REG to avoid duplicate code, code above
    // is kept as an example.

    // Start configure basic timer
    // Enable Timer 6 (Basic Timer)
    REG(RCC_APB1ENR1) |= (1 << 4);

    // CLK = SYSCLK / (PSC + 1)
    // 1kHz = 4MHz / (3999 + 1)
    REG(TIM6_PSC) = 3999;

    // ARR = 999
    // 1 Second = 1 / (1000 * (ARR + 1))
    REG(TIM6_ARR) = 999;

    // Enable interrupts
    REG(TIM6_DIER)= 1;

    // Setup NVIC
    REG(NVIC_ISER1) |= (1 << 22);

    // Enable TIM6
    REG(TIM6_CR1) |= 1;

    for (;;){}
}
