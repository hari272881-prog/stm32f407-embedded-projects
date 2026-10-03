/*
 * console.h
 *
 *  Created on: Sep 30, 2026
 *      Author: hari2
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdint.h>
#include "stm32f407x_usart_driver.h"
/*
 * Sends messages through USART2
 */
void Console_Init(USART_Handle_t *pUSARTHandle);
void Console_SendString(uint8_t *message,uint32_t length);


#endif /* CONSOLE_H_ */
