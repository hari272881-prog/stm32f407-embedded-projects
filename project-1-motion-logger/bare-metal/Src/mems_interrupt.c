/*
 * mems_interrupt.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#include "mems_interrupt.h"
#include "board.h"

#define SYSCFG_BASE_ADDRESS          0x40013800UL
#define EXTI_BASE_ADDRESS            0x40013C00UL

#define RCC_APB2ENR_SYSCFG_ENABLE    (1U << 14)

#define MEMS_INT1_EXTI_LINE          (1U << 0)
#define EXTI0_IRQ_NUMBER             6U
#define MEMS_INTERRUPT_PRIORITY      6U

typedef struct
{
    volatile uint32_t MEMRMP;
    volatile uint32_t PMC;
    volatile uint32_t EXTICR[4];
} MEMS_SYSCFG_RegDef_t;

typedef struct
{
    volatile uint32_t IMR;
    volatile uint32_t EMR;
    volatile uint32_t RTSR;
    volatile uint32_t FTSR;
    volatile uint32_t SWIER;
    volatile uint32_t PR;
} MEMS_EXTI_RegDef_t;

#define MEMS_SYSCFG                  ((MEMS_SYSCFG_RegDef_t *)SYSCFG_BASE_ADDRESS)
#define MEMS_EXTI                    ((MEMS_EXTI_RegDef_t *)EXTI_BASE_ADDRESS)

#define MEMS_NVIC_ISER               ((volatile uint32_t *)0xE000E100UL)
#define MEMS_NVIC_IPR                ((volatile uint8_t *)0xE000E400UL)

volatile uint32_t g_memsInt1InterruptCount = 0U;

static uint32_t g_memsInt1ProcessedCount = 0U;

static void MEMS_EnableIRQ(uint8_t irqNumber, uint8_t priority)
{
    MEMS_NVIC_IPR[irqNumber] = (uint8_t)(priority << 4U);
    MEMS_NVIC_ISER[irqNumber / 32U] = (1U << (irqNumber % 32U));
}

void MEMS_INT1_EXTI_Init(void)
{
    GPIO_Handle_t interruptPin = {0};

    /*
     * LIS3DSH INT1 is physically connected to STM32 PE0.
     */
    GPIO_PeriClockControl(GPIOE, ENABLE);

    interruptPin.pGPIOx = GPIOE;
    interruptPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    interruptPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    interruptPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&interruptPin);

    /*
     * Enable the STM32 SYSCFG peripheral clock.
     */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFG_ENABLE;
    (void)RCC->APB2ENR;

    /*
     * Disable EXTI0 while configuring it.
     */
    MEMS_EXTI->IMR &= ~MEMS_INT1_EXTI_LINE;

    /*
     * EXTICR1 EXTI0[3:0] = 0100 selects GPIO port E.
     */
    MEMS_SYSCFG->EXTICR[0] &= ~(0x0FU << 0U);
    MEMS_SYSCFG->EXTICR[0] |=  (0x04U << 0U);

    /*
     * LIS3DSH interrupt is active high, so detect a
     * rising edge only.
     */
    MEMS_EXTI->RTSR |= MEMS_INT1_EXTI_LINE;
    MEMS_EXTI->FTSR &= ~MEMS_INT1_EXTI_LINE;

    /*
     * Clear any old pending EXTI0 event by writing one.
     */
    MEMS_EXTI->PR = MEMS_INT1_EXTI_LINE;

    g_memsInt1InterruptCount = 0U;
    g_memsInt1ProcessedCount = 0U;

    /*
     * Unmask EXTI0 and enable it in the NVIC.
     */
    MEMS_EXTI->IMR |= MEMS_INT1_EXTI_LINE;

    MEMS_EnableIRQ(EXTI0_IRQ_NUMBER, MEMS_INTERRUPT_PRIORITY);
}

uint8_t MEMS_INT1_TakeEvent(void)
{
    if (g_memsInt1ProcessedCount == g_memsInt1InterruptCount)
    {
        return 0U;
    }

    g_memsInt1ProcessedCount++;

    return 1U;
}

void EXTI0_IRQHandler(void)
{
    if ((MEMS_EXTI->PR & MEMS_INT1_EXTI_LINE) != 0U)
    {
        /*
         * Clear the STM32 EXTI pending flag first.
         */
        MEMS_EXTI->PR = MEMS_INT1_EXTI_LINE;

        /*
         * Do not perform SPI or UART work inside the ISR.
         * Only record the event.
         */
        g_memsInt1InterruptCount++;
    }
}
