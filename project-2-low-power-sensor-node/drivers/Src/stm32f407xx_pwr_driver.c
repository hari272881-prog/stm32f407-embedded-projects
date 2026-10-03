/*
 * stm32f407xx_pwr_driver.c
 *
 *  Created on: Oct 3, 2026
 *      Author: hari2
 */


#include "stm32f407xx_pwr_driver.h"

void PWR_EnterSleepMode(void)
{
    SCB_SCR &= ~SCB_SCR_SLEEPONEXIT;
    SCB_SCR &= ~SCB_SCR_SLEEPDEEP;

    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("wfi");
    __asm volatile ("isb" ::: "memory");
}

void PWR_EnterStopMode(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;

    PWR->CR &= ~PWR_CR_PDDS;
    PWR->CR |= PWR_CR_LPDS;
    PWR->CR |= PWR_CR_FPDS;
    PWR->CR |= PWR_CR_CWUF;

    SCB_SCR &= ~SCB_SCR_SLEEPONEXIT;
    SCB_SCR |= SCB_SCR_SLEEPDEEP;

    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("wfi");
    __asm volatile ("isb" ::: "memory");

    SCB_SCR &= ~SCB_SCR_SLEEPDEEP;
}
