/*
 * stm32f407xx_rtc_driver.c
 *
 *  Created on: Oct 3, 2026
 *      Author: hari2
 */


#include "stm32f407xx_rtc_driver.h"

#define RTC_CLOCK_STARTUP_TIMEOUT    1000000U
#define RTC_WRITE_TIMEOUT            1000000U

static volatile uint32_t g_rtcWakeupCount = 0U;

uint8_t RTC_ClockSource_Init(void)
{
    uint32_t timeout = RTC_CLOCK_STARTUP_TIMEOUT;

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;

    PWR->CR |= PWR_CR_DBP;

    if ((PWR->CR & PWR_CR_DBP) == 0U)
    {
        return 0U;
    }

    RCC->CSR |= RCC_CSR_LSION;

    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U)
    {
        if (timeout == 0U)
        {
            PWR->CR &= ~PWR_CR_DBP;
            return 0U;
        }

        timeout--;
    }

    RCC->BDCR |= RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    RCC->BDCR &= ~RCC_BDCR_RTCSEL_MASK;
    RCC->BDCR |= RCC_BDCR_RTCSEL_LSI;
    RCC->BDCR |= RCC_BDCR_RTCEN;

    if ((RCC->BDCR & RCC_BDCR_RTCSEL_MASK) != RCC_BDCR_RTCSEL_LSI)
    {
        PWR->CR &= ~PWR_CR_DBP;
        return 0U;
    }

    if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0U)
    {
        PWR->CR &= ~PWR_CR_DBP;
        return 0U;
    }

    PWR->CR &= ~PWR_CR_DBP;

    return 1U;
}
uint8_t RTC_WakeupTimer_Init(uint16_t reloadValue)
{
    uint32_t timeout = RTC_WRITE_TIMEOUT;

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;

    PWR->CR |= PWR_CR_DBP;

    if ((PWR->CR & PWR_CR_DBP) == 0U)
    {
        return 0U;
    }

    RTC->WPR = RTC_WPR_UNLOCK_KEY1;
    RTC->WPR = RTC_WPR_UNLOCK_KEY2;

    RTC->CR &= ~RTC_CR_WUTIE;
    RTC->CR &= ~RTC_CR_WUTE;

    while ((RTC->ISR & RTC_ISR_WUTWF) == 0U)
    {
        if (timeout == 0U)
        {
            RTC->WPR = RTC_WPR_LOCK_KEY;
            PWR->CR &= ~PWR_CR_DBP;
            return 0U;
        }

        timeout--;
    }

    RTC->CR &= ~RTC_CR_WUCKSEL_MASK;
    RTC->CR |= RTC_CR_WUCKSEL_RTCCLK_DIV16;
    RTC->WUTR = reloadValue;

    if ((RTC->WUTR & 0xFFFFU) != reloadValue)
    {
        RTC->WPR = RTC_WPR_LOCK_KEY;
        PWR->CR &= ~PWR_CR_DBP;
        return 0U;
    }

    RTC->ISR &= ~RTC_ISR_WUTF;

    *NVIC_ICER0 = (1U << IRQ_NO_RTC_WKUP);
    *NVIC_ICPR0 = (1U << IRQ_NO_RTC_WKUP);

    EXTI->IMR |= EXTI_LINE_22;
    EXTI->RTSR |= EXTI_LINE_22;
    EXTI->FTSR &= ~EXTI_LINE_22;
    EXTI->PR = EXTI_LINE_22;

    g_rtcWakeupCount = 0U;

    RTC->CR |= RTC_CR_WUTIE;
    RTC->CR |= RTC_CR_WUTE;

    if ((RTC->CR & (RTC_CR_WUTIE | RTC_CR_WUTE)) != (RTC_CR_WUTIE | RTC_CR_WUTE))
    {
        RTC->WPR = RTC_WPR_LOCK_KEY;
        PWR->CR &= ~PWR_CR_DBP;
        return 0U;
    }

    *NVIC_ISER0 = (1U << IRQ_NO_RTC_WKUP);

    RTC->WPR = RTC_WPR_LOCK_KEY;
    PWR->CR &= ~PWR_CR_DBP;

    return 1U;
}

uint32_t RTC_GetWakeupCount(void)
{
    return g_rtcWakeupCount;
}

void RTC_WKUP_IRQHandler(void)
{
    if ((RTC->ISR & RTC_ISR_WUTF) != 0U)
    {
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        (void)RCC->APB1ENR;

        PWR->CR |= PWR_CR_DBP;

        RTC->WPR = RTC_WPR_UNLOCK_KEY1;
        RTC->WPR = RTC_WPR_UNLOCK_KEY2;

        RTC->CR &= ~RTC_CR_WUTIE;
        RTC->ISR &= ~RTC_ISR_WUTF;

        EXTI->PR = EXTI_LINE_22;
        PWR->CR |= PWR_CR_CWUF;

        RTC->CR |= RTC_CR_WUTIE;

        RTC->WPR = RTC_WPR_LOCK_KEY;
        PWR->CR &= ~PWR_CR_DBP;

        g_rtcWakeupCount++;
    }
}
