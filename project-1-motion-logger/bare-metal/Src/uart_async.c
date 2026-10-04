/*
 * uart_async.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#include "uart_async.h"
#include "stm32f407xx.h"
#include <string.h>

#define UART2_RX_BUFFER_SIZE      256U
#define UART2_RX_BUFFER_MASK      (UART2_RX_BUFFER_SIZE - 1U)

#define UART2_TX_QUEUE_DEPTH      8U
#define UART2_TX_QUEUE_MASK       (UART2_TX_QUEUE_DEPTH - 1U)
#define UART2_TX_FRAME_MAX_SIZE   160U

#define DMA1_BASE_ADDRESS         0x40026000UL
#define DMA1_STREAM6_ADDRESS      0x400260A0UL

#define DMA_STREAM_CR_EN          (1U << 0)
#define DMA_STREAM_CR_DMEIE       (1U << 1)
#define DMA_STREAM_CR_TEIE        (1U << 2)
#define DMA_STREAM_CR_TCIE        (1U << 4)
#define DMA_STREAM_CR_DIR_M2P     (1U << 6)
#define DMA_STREAM_CR_MINC        (1U << 10)
#define DMA_STREAM_CR_PRIORITY    (1U << 16)
#define DMA_STREAM_CR_CHANNEL4    (4U << 25)

#define DMA_HISR_FEIF6            (1U << 16)
#define DMA_HISR_DMEIF6           (1U << 18)
#define DMA_HISR_TEIF6            (1U << 19)
#define DMA_HISR_HTIF6            (1U << 20)
#define DMA_HISR_TCIF6            (1U << 21)

#define DMA_STREAM6_ERROR_FLAGS   (DMA_HISR_DMEIF6 | DMA_HISR_TEIF6)

#define DMA_STREAM6_ALL_FLAGS     (DMA_HISR_FEIF6  | \
                                   DMA_HISR_DMEIF6 | \
                                   DMA_HISR_TEIF6  | \
                                   DMA_HISR_HTIF6  | \
                                   DMA_HISR_TCIF6)

#define USART_STATUS_PE           (1U << 0)
#define USART_STATUS_FE           (1U << 1)
#define USART_STATUS_NE           (1U << 2)
#define USART_STATUS_ORE          (1U << 3)
#define USART_STATUS_RXNE         (1U << 5)
#define USART_STATUS_TC           (1U << 6)

#define USART_ERROR_FLAGS         (USART_STATUS_PE  | \
                                   USART_STATUS_FE  | \
                                   USART_STATUS_NE  | \
                                   USART_STATUS_ORE)

#define USART_CR1_RXNEIE_ENABLE   (1U << 5)
#define USART_CR3_EIE_ENABLE      (1U << 0)
#define USART_CR3_DMAT_ENABLE     (1U << 7)

#define RCC_AHB1ENR_DMA1_ENABLE   (1U << 21)

#define DMA1_STREAM6_IRQ_NUMBER   17U
#define USART2_IRQ_NUMBER         38U
#define UART_INTERRUPT_PRIORITY   6U

typedef struct
{
    volatile uint32_t LISR;
    volatile uint32_t HISR;
    volatile uint32_t LIFCR;
    volatile uint32_t HIFCR;
} UART_DMA_Controller_t;

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t NDTR;
    volatile uint32_t PAR;
    volatile uint32_t M0AR;
    volatile uint32_t M1AR;
    volatile uint32_t FCR;
} UART_DMA_Stream_t;

typedef struct
{
    uint16_t length;
    uint8_t data[UART2_TX_FRAME_MAX_SIZE];
} UART2_TxFrame_t;

#define UART_DMA1                 ((UART_DMA_Controller_t *)DMA1_BASE_ADDRESS)
#define UART_DMA1_STREAM6         ((UART_DMA_Stream_t *)DMA1_STREAM6_ADDRESS)

#define UART_NVIC_ISER            ((volatile uint32_t *)0xE000E100UL)
#define UART_NVIC_IPR             ((volatile uint8_t *)0xE000E400UL)

static uint8_t g_rxBuffer[UART2_RX_BUFFER_SIZE];
static volatile uint16_t g_rxHead;
static volatile uint16_t g_rxTail;

static UART2_TxFrame_t g_txQueue[UART2_TX_QUEUE_DEPTH];
static volatile uint8_t g_txHead;
static volatile uint8_t g_txTail;
static volatile uint8_t g_txActive;

static UART2_AsyncStats_t g_uartStats;

static void UART2_DataMemoryBarrier(void)
{
    __asm volatile ("dmb" ::: "memory");
}

static uint32_t UART2_SaveInterruptState(void)
{
    uint32_t interruptState;

    __asm volatile ("MRS %0, PRIMASK" : "=r" (interruptState));
    __asm volatile ("cpsid i" ::: "memory");

    return interruptState;
}

static void UART2_RestoreInterruptState(uint32_t interruptState)
{
    if ((interruptState & 1U) == 0U)
    {
        __asm volatile ("cpsie i" ::: "memory");
    }
}

static void UART2_EnableIRQ(uint8_t irqNumber, uint8_t priority)
{
    UART_NVIC_IPR[irqNumber] = (uint8_t)(priority << 4U);
    UART_NVIC_ISER[irqNumber / 32U] = (1U << (irqNumber % 32U));
}

static void UART2_DMA_ClearFlags(void)
{
    UART_DMA1->HIFCR = DMA_STREAM6_ALL_FLAGS;
}

static void UART2_DMA_DisableStream(void)
{
    UART_DMA1_STREAM6->CR &= ~DMA_STREAM_CR_EN;

    while ((UART_DMA1_STREAM6->CR & DMA_STREAM_CR_EN) != 0U)
    {
    }
}

static void UART2_DMA_StartNext(void)
{
    UART2_TxFrame_t *frame;

    /*
     * Do not restart DMA while another frame is active.
     */
    if (g_txActive != 0U)
    {
        return;
    }

    /*
     * The transmission queue is empty.
     */
    if (g_txTail == g_txHead)
    {
        return;
    }

    frame = &g_txQueue[g_txTail];

    UART2_DMA_DisableStream();
    UART2_DMA_ClearFlags();

    UART_DMA1_STREAM6->M0AR = (uint32_t)(uintptr_t)frame->data;
    UART_DMA1_STREAM6->NDTR = frame->length;

    /*
     * Make the frame contents and DMA register writes visible
     * before enabling the DMA stream.
     */
    UART2_DataMemoryBarrier();

    g_txActive = 1U;
    UART_DMA1_STREAM6->CR |= DMA_STREAM_CR_EN;
}

void UART2_AsyncInit(void)
{
    g_rxHead = 0U;
    g_rxTail = 0U;

    g_txHead = 0U;
    g_txTail = 0U;
    g_txActive = 0U;

    memset((void *)&g_uartStats, 0, sizeof(g_uartStats));

    /*
     * Enable the DMA1 peripheral clock.
     */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1_ENABLE;
    (void)RCC->AHB1ENR;

    /*
     * DMA streams must be disabled before configuration.
     */
    UART2_DMA_DisableStream();

    /*
     * Clear any old flags belonging to DMA1 Stream 6.
     */
    UART2_DMA_ClearFlags();

    /*
     * Configure DMA1 Stream 6:
     *
     * Channel 4
     * Memory-to-peripheral
     * Memory address increment
     * 8-bit peripheral data
     * 8-bit memory data
     * Medium priority
     * Transfer-complete interrupt
     * Transfer-error interrupt
     * Direct-mode error interrupt
     */
    UART_DMA1_STREAM6->CR = DMA_STREAM_CR_CHANNEL4 |
                            DMA_STREAM_CR_PRIORITY |
                            DMA_STREAM_CR_MINC |
                            DMA_STREAM_CR_DIR_M2P |
                            DMA_STREAM_CR_TCIE |
                            DMA_STREAM_CR_TEIE |
                            DMA_STREAM_CR_DMEIE;

    UART_DMA1_STREAM6->NDTR = 0U;
    UART_DMA1_STREAM6->PAR = (uint32_t)(uintptr_t)&USART2->DR;
    UART_DMA1_STREAM6->M0AR = 0U;
    UART_DMA1_STREAM6->M1AR = 0U;

    /*
     * Direct mode enabled and FIFO interrupt disabled.
     */
    UART_DMA1_STREAM6->FCR = 0U;

    /*
     * USART2 requests a DMA transfer whenever its TX data
     * register becomes empty.
     */
    USART2->CR3 |= USART_CR3_DMAT_ENABLE;

    /*
     * Enable USART error and receive-data interrupts.
     */
    USART2->CR3 |= USART_CR3_EIE_ENABLE;
    USART2->CR1 |= USART_CR1_RXNEIE_ENABLE;

    UART2_EnableIRQ(DMA1_STREAM6_IRQ_NUMBER, UART_INTERRUPT_PRIORITY);
    UART2_EnableIRQ(USART2_IRQ_NUMBER, UART_INTERRUPT_PRIORITY);
}

uint32_t UART2_AsyncRxAvailable(void)
{
    return (uint32_t)((g_rxHead - g_rxTail) & UART2_RX_BUFFER_MASK);
}

uint8_t UART2_AsyncReadByte(uint8_t *data)
{
    if (data == 0)
    {
        return 0U;
    }

    if (g_rxTail == g_rxHead)
    {
        return 0U;
    }

    *data = g_rxBuffer[g_rxTail];

    UART2_DataMemoryBarrier();

    g_rxTail = (uint16_t)((g_rxTail + 1U) & UART2_RX_BUFFER_MASK);

    return 1U;
}

uint8_t UART2_AsyncQueueTx(const uint8_t *data, uint16_t length)
{
    UART2_TxFrame_t *frame;
    uint8_t nextHead;
    uint32_t interruptState;

    if ((data == 0) || (length == 0U) || (length > UART2_TX_FRAME_MAX_SIZE))
    {
        return 0U;
    }

    /*
     * Protect the queue while publishing a new frame.
     */
    interruptState = UART2_SaveInterruptState();

    nextHead = (uint8_t)((g_txHead + 1U) & UART2_TX_QUEUE_MASK);

    if (nextHead == g_txTail)
    {
        g_uartStats.txQueueFull++;
        UART2_RestoreInterruptState(interruptState);
        return 0U;
    }

    frame = &g_txQueue[g_txHead];

    memcpy(frame->data, data, length);
    frame->length = length;

    /*
     * Publish the completed queue entry only after its contents
     * have been copied.
     */
    UART2_DataMemoryBarrier();

    g_txHead = nextHead;
    g_uartStats.txFramesQueued++;

    /*
     * If DMA is idle, this starts the newly queued frame.
     * If DMA is already active, the interrupt handler will
     * start this frame later.
     */
    UART2_DMA_StartNext();

    UART2_RestoreInterruptState(interruptState);

    return 1U;
}

uint8_t UART2_AsyncTxIdle(void)
{
    if (g_txActive != 0U)
    {
        return 0U;
    }

    if (g_txTail != g_txHead)
    {
        return 0U;
    }

    if ((UART_DMA1_STREAM6->CR & DMA_STREAM_CR_EN) != 0U)
    {
        return 0U;
    }

    if ((USART2->SR & USART_STATUS_TC) == 0U)
    {
        return 0U;
    }

    return 1U;
}

const UART2_AsyncStats_t *UART2_AsyncGetStats(void)
{
    return &g_uartStats;
}

void USART2_IRQHandler(void)
{
    uint32_t status;
    uint8_t receivedByte;
    uint16_t nextHead;

    /*
     * Reading SR followed by DR clears the STM32F4 USART
     * receive-error conditions.
     */
    status = USART2->SR;

    if ((status & USART_ERROR_FLAGS) != 0U)
    {
        receivedByte = (uint8_t)USART2->DR;
        (void)receivedByte;

        g_uartStats.uartErrors++;
        return;
    }

    if ((status & USART_STATUS_RXNE) != 0U)
    {
        receivedByte = (uint8_t)USART2->DR;

        nextHead = (uint16_t)((g_rxHead + 1U) & UART2_RX_BUFFER_MASK);

        if (nextHead == g_rxTail)
        {
            g_uartStats.rxOverflow++;
        }
        else
        {
            g_rxBuffer[g_rxHead] = receivedByte;

            UART2_DataMemoryBarrier();

            g_rxHead = nextHead;
            g_uartStats.rxBytes++;
        }
    }
}

void DMA1_Stream6_IRQHandler(void)
{
    uint32_t status;

    status = UART_DMA1->HISR;

    /*
     * Stop the completed or failed stream before modifying
     * its address and length registers.
     */
    UART2_DMA_DisableStream();

    /*
     * Clear every Stream 6 interrupt flag.
     */
    UART2_DMA_ClearFlags();

    if ((status & DMA_STREAM6_ERROR_FLAGS) != 0U)
    {
        /*
         * The current frame did not complete successfully.
         */
        g_uartStats.dmaErrors++;
        g_uartStats.txFramesDropped++;
    }
    else if ((status & DMA_HISR_TCIF6) != 0U)
    {
        /*
         * The complete frame was transferred to USART2.
         */
        g_uartStats.dmaCompletions++;
    }
    else
    {
        /*
         * No fatal error or transfer-complete event was found.
         */
        return;
    }

    /*
     * The current DMA transaction is no longer active.
     */
    g_txActive = 0U;

    /*
     * Remove the completed or failed frame from the queue.
     */
    g_txTail = (uint8_t)((g_txTail + 1U) & UART2_TX_QUEUE_MASK);

    UART2_DataMemoryBarrier();

    /*
     * Start the next queued frame, if one exists.
     */
    UART2_DMA_StartNext();
}
