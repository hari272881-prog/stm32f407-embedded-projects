/*
 * stm32f407xx_pwr_driver.h
 *
 *  Created on: Oct 3, 2026
 *      Author: hari2
 */

#ifndef INC_STM32F407XX_PWR_DRIVER_H_
#define INC_STM32F407XX_PWR_DRIVER_H_

#include "stm32f407xx.h"
/*
 * PWR Control register
 */
#define PWR_CR_LPDS (1U << 0)
#define PWR_CR_PDDS (1U << 1)
#define PWR_CR_CWUF (1U << 2)
#define PWR_CR_FPDS (1U << 9)

#define SCB_SCR_SLEEPONEXIT (1U << 1)
#define SCB_SCR_SLEEPDEEP (1U << 2)


void PWR_EnterSleepMode(void);
void PWR_EnterStopMode(void);

#endif /* INC_STM32F407XX_PWR_DRIVER_H_ */
