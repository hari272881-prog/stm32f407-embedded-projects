/*
 * application.h
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <stdint.h>
#include "stm32f407xx.h"
/*
 * This containsProject logic: read, filter, detect tilt, update LEDs
 */


void App_Init(void);
void App_Run(void);

#endif /* APPLICATION_H_ */
