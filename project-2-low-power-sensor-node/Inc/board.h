/*
 * boeard.h
 *
 *  Created on: Sep 30, 2026
 *      Author: hari2
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>
/*
 * Initializes pins, SPI1, USART2 and LEDs
 */
#include "stm32f407x_gpio_driver.h"
#include "stm32f407xx_spi_driver.h"
#include "stm32f407x_usart_driver.h"

/*
 * LIS3DSH chip-select connection
 *
 * STM32F407 Discovery:
 * PE3 → LIS3DSH CS
 */
#define ACCEL_CS_PORT    GPIOE
#define ACCEL_CS_PIN     GPIO_PIN_NO_3

#define BOARD_LED_PORT    GPIOD

#define BOARD_LED_GREEN   GPIO_PIN_NO_12
#define BOARD_LED_ORANGE  GPIO_PIN_NO_13
#define BOARD_LED_RED     GPIO_PIN_NO_14
#define BOARD_LED_BLUE    GPIO_PIN_NO_15

void Board_LEDs_Init(void);
void Board_LEDs_Off(void);
void Board_LEDs_On(void);
void Board_LED_On(uint8_t ledPin);

void USART2_GPIO_Init(void);
void USART2_InitForTest(USART_Handle_t *p_usart2_handle);
void SPI1_GPIO_Init(void);
void SPI1_InitForAccelerometer(SPI_Handle_t *p_spi1_handle);

#endif /* BOARD_H_ */
