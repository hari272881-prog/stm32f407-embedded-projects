/*
 * uart_async.h
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#ifndef UART_ASYNC_H_
#define UART_ASYNC_H_

#include <stdint.h>

typedef struct
{
    volatile uint32_t rxBytes;
    volatile uint32_t rxOverflow;
    volatile uint32_t uartErrors;
    volatile uint32_t txFramesQueued;
    volatile uint32_t txQueueFull;
    volatile uint32_t dmaCompletions;
    volatile uint32_t dmaErrors;
    volatile uint32_t txFramesDropped;
} UART2_AsyncStats_t;

void UART2_AsyncInit(void);
uint8_t UART2_AsyncReadByte(uint8_t *data);
uint32_t UART2_AsyncRxAvailable(void);
uint8_t UART2_AsyncQueueTx(const uint8_t *data, uint16_t length);
uint8_t UART2_AsyncTxIdle(void);
const UART2_AsyncStats_t *UART2_AsyncGetStats(void);

#endif /* UART_ASYNC_H_ */
