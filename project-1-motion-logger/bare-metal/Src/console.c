/*
 * console.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */


#include "console.h"

/*
 * Pointer to the USART handle owned by main.c.
 * Private to console.c.
 */
static USART_Handle_t *s_pConsoleUSART = 0;

void Console_Init(USART_Handle_t *pUSARTHandle)
{
    s_pConsoleUSART = pUSARTHandle;
}

void Console_SendString(uint8_t *message,uint32_t length)
{
    if ((s_pConsoleUSART == 0) ||(message == 0) ||(length == 0U))
    {
        return;
    }

    USART_SendData(s_pConsoleUSART,message,length);
}
