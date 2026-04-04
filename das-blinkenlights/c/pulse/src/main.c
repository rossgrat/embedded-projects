#include <stdint.h>

#define RCC_BASE            0x40021000
#define GPIOB_BASE          0x48000400
// Timers are driven by SYSCLK. The default source for SYSCLK is MSI (Multi-speed Internal Oscillator)
// at 4MHz
#define TIM6_BASE           0x40001000
#define TIM4_BASE           0x40000800

// RCC_AHB2ENR = Reset Clock Control, Advance High-Speed Bus 2, Enable Register
#define RCC_AHB2ENR_OFFSET  0x4C
#define RCC_AHB2ENR         (RCC_BASE + RCC_AHB2ENR_OFFSET)

// RCC_AHB2ENR = Reset Clock Control, Advance High-Speed Bus 2, Enable Register
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
#define TIMX_CCMR1          0x18                                // Compare/Capture Mode Register 1
#define TIMX_CCER           0x20                                // Compare/Capture Enable Register
// #define TIMX_CNT            0x24                               // Counter
#define TIMX_PSC            0x28                                // Prescaler
#define TIMX_ARR            0x2C                                // Auto-Reload Register
#define TIMX_CCR2           0x38                                // Compare/Capture Register 2

#define TIM6_CR1            (TIM6_BASE + TIMX_CR1)
#define TIM6_SR             (TIM6_BASE + TIMX_SR)
#define TIM6_DIER           (TIM6_BASE + TIMX_DIER)
#define TIM6_PSC            (TIM6_BASE + TIMX_PSC)
#define TIM6_ARR            (TIM6_BASE + TIMX_ARR)

#define TIM4_CCMR1          (TIM4_BASE + TIMX_CCMR1)
#define TIM4_CCER           (TIM4_BASE + TIMX_CCER)
#define TIM4_PSC            (TIM4_BASE + TIMX_PSC)
#define TIM4_ARR            (TIM4_BASE + TIMX_ARR)
#define TIM4_CR1            (TIM4_BASE + TIMX_CR1)
#define TIM4_CCR2           (TIM4_BASE + TIMX_CCR2)


// ARM Core functionality, including the NVIC controller, is distinct from
// ST designed peripherals
#define NVIC_BASE               0xE000E000
#define NVIC_ISERX_OFFSET       0x100
#define NVIC_ISER1_OFFSET       0x04
#define NVIC_ISER1              (NVIC_BASE + NVIC_ISERX_OFFSET + NVIC_ISER1_OFFSET)

// The code below introduces a way of avoiding the verbose declaration
#define REG(addr) (*(volatile uint32_t*)(addr))

static int lightDirection = 1;

void adjustLightPWM() {
    volatile uint32_t* tim4_ccr2 = (volatile uint32_t*)TIM4_CCR2;
    if (lightDirection) {
        (*tim4_ccr2)++;
        if (*tim4_ccr2 == 100) {
            lightDirection = 0;
        }
    } else {
        (*tim4_ccr2)--;
        if (*tim4_ccr2 == 0) {
            lightDirection = 1;
        }
    }

    REG(TIM6_SR) = 0; // Reset Interrupt Register
    (void)REG(TIM6_SR); // Read Interrupt Register
}

void main() {

    // Start configure blue light
    // Enable Port B (Pins GPIOB1, GPIOB2, etc.)
    REG(RCC_AHB2ENR) |= (1 << 1);

    // Set Port B mode to output (10 = Alternate Function)
    // Reset pin values and set mode
    REG(GPIOB_MODER) &= ~(3 << 14);
    REG(GPIOB_MODER) |= (2 << 14);
    // Configure Alternate Function to be TIM4
    // Reset pin and set mode
    REG(GPIOB_AFRL) &= ~(15 << 28);
    REG(GPIOB_AFRL) |= (2 << 28);


    // Start configure basic timer
    // Enable Timer 6 (Basic Timer)
    REG(RCC_APB1ENR1) |= (1 << 4);
    // Enable Timer 4 (General Purpose Timer)
    REG(RCC_APB1ENR1) |= (1 << 2);
    // CLK = SYSCLK / (PSC + 1)
    // 1kHz = 4MHz / (3999 + 1)
    REG(TIM6_PSC) = 3999;
    // ARR = 999
    // 200 Hz  - Interrupt fires 200 times per second for the full pulse
    // There are 100 steps up and 100 steps down in a full pulse
    REG(TIM6_ARR) = 4;
    // Enable interrupts
    REG(TIM6_DIER) = 1;

    // Start configure general purpose (PWM) timer (Timer 4 Channel 2)
    // We use OC2M (Output Compare 2 Mode) because we are on channel 2
    REG(TIM4_CCMR1) |= (6 << 12);
    // Enable Compare and Capture on channel 2
    REG(TIM4_CCER) |= (1 << 4);
    // PSC and ARR - We want a refresh rate of 1kHz
    // ARR controls the potential steps in the duty cycle. Since we're
    // utilizing the compare feature of the timer (as PWM), this means that
    // when the counter is > CCR2, light is off, when counter < CCR2, light
    // is on.
    REG(TIM4_PSC) = 39;
    REG(TIM4_ARR) = 99;
    // Light starts dark
    REG(TIM4_CCR2) = 50;

    // Setup NVIC
    REG(NVIC_ISER1) |= (1 << 22);
    // Enable TIM6
    REG(TIM6_CR1) |= 1;
    // Enable TIM4
    REG(TIM4_CR1) |= 1;

    for (;;){}
}
