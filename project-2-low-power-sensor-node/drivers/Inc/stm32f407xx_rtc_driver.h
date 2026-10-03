/*
 * rtc.h
 *
 *  Created on: Oct 3, 2026
 *      Author: hari2
 */

#ifndef INC_STM32F407XX_RTC_DRIVER_H_
#define INC_STM32F407XX_RTC_DRIVER_H_

#include "stm32f407xx.h"

uint8_t RTC_ClockSource_Init(void);
uint8_t RTC_WakeupTimer_Init(uint16_t reloadValue);
uint32_t RTC_GetWakeupCount(void);
#endif /* INC_STM32F407XX_RTC_DRIVER_H_ */
