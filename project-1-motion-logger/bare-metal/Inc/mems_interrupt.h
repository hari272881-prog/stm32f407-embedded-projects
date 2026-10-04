/*
 * mems_interrupt.h
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#ifndef INC_MEMS_INTERRUPT_H_
#define INC_MEMS_INTERRUPT_H_

#include <stdint.h>

extern volatile uint32_t g_memsInt1InterruptCount;

void MEMS_INT1_EXTI_Init(void);
uint8_t MEMS_INT1_TakeEvent(void);

#endif
