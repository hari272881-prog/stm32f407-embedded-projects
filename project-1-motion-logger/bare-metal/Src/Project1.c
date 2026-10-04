#include "board.h"
#include "lis3dsh.h"
#include "orientation.h"
#include "motion_detection.h"
#include "uart_async.h"
#include "cli.h"
#include "mems_interrupt.h"
#include <stdio.h>

static USART_Handle_t g_usart2_handle = {0};
static SPI_Handle_t g_spi1_handle = {0};

LIS3DSH_Data_t g_acceleration = {0};
LIS3DSH_MgData_t g_acceleration_mg = {0};

volatile Orientation_t g_orientation = ORIENTATION_UNKNOWN;
volatile TiltDirection_t g_tiltDirection = TILT_LEVEL;

volatile uint32_t g_sample_count = 0U;
volatile uint32_t g_tapCount = 0U;

volatile uint32_t g_framesGenerated = 0U;
volatile uint32_t g_framesDropped = 0U;
volatile uint32_t g_nextSequence = 0U;
volatile uint32_t g_rxBytesConsumed = 0U;

volatile uint16_t g_sensorRateHz = 100U;
volatile uint8_t g_telemetryEnabled = 1U;

volatile uint8_t g_lastTapInterruptSource = 0U;

const UART2_AsyncStats_t *g_uartStats = 0;

static void Application_ShowTiltLED(TiltDirection_t tilt)
{
    Board_LEDs_Off();

    if ((tilt & TILT_LEFT) != 0U)
    {
        Board_LED_On(BOARD_LED_GREEN);
    }

    if ((tilt & TILT_RIGHT) != 0U)
    {
        Board_LED_On(BOARD_LED_RED);
    }

    if ((tilt & TILT_FORWARD) != 0U)
    {
        Board_LED_On(BOARD_LED_ORANGE);
    }

    if ((tilt & TILT_BACKWARD) != 0U)
    {
        Board_LED_On(BOARD_LED_BLUE);
    }
}

int main(void)
{
    char message[160];
    uint8_t receivedByte;
    uint32_t printAccumulator = 0U;
    uint32_t tapFlashCounter = 0U;
    uint32_t sequenceNumber;
    uint32_t transmitLength;
    int messageLength;

    CLI_Context_t cliContext;

    uint8_t successMessage[] =
        "LIS3DSH and hardware double-tap interrupt initialized\r\n";

    uint8_t sensorFailureMessage[] =
        "LIS3DSH initialization failed\r\n";

    uint8_t interruptFailureMessage[] =
        "LIS3DSH double-tap interrupt initialization failed\r\n";

    /*
     * Configure USART2 and asynchronous UART operation.
     */
    USART2_GPIO_Init();
    USART2_InitForTest(&g_usart2_handle);

    UART2_AsyncInit();
    g_uartStats = UART2_AsyncGetStats();

    /*
     * Configure SPI1 for the onboard LIS3DSH.
     */
    SPI1_GPIO_Init();
    SPI1_InitForAccelerometer(&g_spi1_handle);

    /*
     * Configure the four onboard LEDs.
     */
    Board_LEDs_Init();

    /*
     * Initialize and verify the LIS3DSH.
     */
    if (LIS3DSH_Init() == 0U)
    {
        (void)UART2_AsyncQueueTx(sensorFailureMessage,
                                 sizeof(sensorFailureMessage) - 1U);

        Board_LED_On(BOARD_LED_RED);

        while (1)
        {
        }
    }

    /*
     * Configure LIS3DSH State Machine 1 for hardware
     * double-tap recognition on the INT1 pin.
     *
     * The ST state-machine timing is configured for the selected ODR.
     */
    if (LIS3DSH_EnableDoubleTapInterrupt(g_sensorRateHz) == 0U)
    {
        (void)UART2_AsyncQueueTx(interruptFailureMessage,
                                 sizeof(interruptFailureMessage) - 1U);

        Board_LED_On(BOARD_LED_RED);

        while (1)
        {
        }
    }


    /*
     * Clear any old latched LIS3DSH interrupt before
     * enabling STM32 EXTI0.
     */
    g_lastTapInterruptSource = LIS3DSH_ClearTapInterrupt();

    /*
     * Configure PE0 as EXTI0:
     *
     * LIS3DSH INT1 → PE0 → EXTI0.
     */
    MEMS_INT1_EXTI_Init();

    (void)UART2_AsyncQueueTx(successMessage,
                             sizeof(successMessage) - 1U);

    /*
     * Initialize software orientation and tilt detection.
     *
     * Software tap checking is no longer used.
     */
    Orientation_Init();
    MotionDetection_Init();

    /*
     * Give the CLI access to the current application state.
     */
    cliContext.pAcceleration = &g_acceleration_mg;
    cliContext.pOrientation = &g_orientation;
    cliContext.pTiltDirection = &g_tiltDirection;
    cliContext.pTapCount = &g_tapCount;
    cliContext.pSampleCount = &g_sample_count;
    cliContext.pFramesGenerated = &g_framesGenerated;
    cliContext.pFramesDropped = &g_framesDropped;
    cliContext.pSensorRateHz = &g_sensorRateHz;
    cliContext.pTelemetryEnabled = &g_telemetryEnabled;

    CLI_Init(&cliContext);

    while (1)
    {
        /*
         * Process all received UART bytes.
         */
        while (UART2_AsyncReadByte(&receivedByte) != 0U)
        {
            g_rxBytesConsumed++;
            CLI_ProcessByte(receivedByte);
        }

        /*
         * Process hardware double-tap interrupt events.
         *
         * EXTI0_IRQHandler() only records the event.
         * SPI access and application processing happen here.
         */
        while (MEMS_INT1_TakeEvent() != 0U)
        {
            /*
             * Reading OUTS1 releases the latched LIS3DSH
             * INT1 output so another rising edge can occur.
             */
            g_lastTapInterruptSource =
                LIS3DSH_ClearTapInterrupt();

            g_tapCount++;

            /*
             * Flash all LEDs for approximately 100 ms.
             */
            tapFlashCounter =
                ((uint32_t)g_sensorRateHz + 9U) / 10U;
        }

        /*
         * Continue reading accelerometer samples normally.
         */
        if (LIS3DSH_IsDataReady() != 0U)
        {
            LIS3DSH_ReadXYZ(&g_acceleration);

            LIS3DSH_ConvertRawToMg(&g_acceleration,
                                   &g_acceleration_mg);

            g_orientation =
                Orientation_Update(&g_acceleration_mg);

            g_tiltDirection =
                MotionDetection_UpdateTilt(&g_acceleration_mg);

            g_sample_count++;

            /*
             * Do not call MotionDetection_CheckTap() here.
             * Tap detection is now performed by LIS3DSH
             * State Machine 1 and EXTI0.
             */

            if (tapFlashCounter > 0U)
            {
                Board_LEDs_On();
                tapFlashCounter--;
            }
            else
            {
                Application_ShowTiltLED(g_tiltDirection);
            }

            /*
             * Periodic telemetry can be paused without
             * stopping sensor acquisition or motion processing.
             */
            if (g_telemetryEnabled != 0U)
            {
                /*
                 * Maintain approximately ten telemetry frames
                 * per second.
                 */
                printAccumulator += 10U;

                if (printAccumulator >=
                    (uint32_t)g_sensorRateHz)
                {
                    printAccumulator -=
                        (uint32_t)g_sensorRateHz;

                    sequenceNumber = g_nextSequence;
                    g_nextSequence++;
                    g_framesGenerated++;

                    messageLength = snprintf(
                        message,
                        sizeof(message),
                        "SEQ=%lu,X=%ld,Y=%ld,Z=%ld,O=%s,T=%s,Taps=%lu,IRQ=%lu,Drop=%lu\r\n",
                        (unsigned long)sequenceNumber,
                        (long)g_acceleration_mg.x,
                        (long)g_acceleration_mg.y,
                        (long)g_acceleration_mg.z,
                        Orientation_GetName(g_orientation),
                        MotionDetection_GetTiltName(
                            g_tiltDirection),
                        (unsigned long)g_tapCount,
                        (unsigned long)
                            g_memsInt1InterruptCount,
                        (unsigned long)g_framesDropped);

                    if ((messageLength > 0) &&
                        ((uint32_t)messageLength <
                         sizeof(message)))
                    {
                        transmitLength =
                            (uint32_t)messageLength;

                        if (UART2_AsyncQueueTx(
                                (uint8_t *)message,
                                (uint16_t)transmitLength) == 0U)
                        {
                            g_framesDropped++;
                        }
                    }
                    else
                    {
                        g_framesDropped++;
                    }
                }
            }
            else
            {
                /*
                 * Begin a fresh telemetry interval after
                 * monitoring is enabled again.
                 */
                printAccumulator = 0U;
            }
        }
    }
}
