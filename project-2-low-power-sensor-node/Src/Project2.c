#include "console.h"
#include "board.h"
#include "lis3dsh.h"
#include "orientation.h"
#include "motion_detection.h"
#include <stdio.h>

#define ACTIVE_CURRENT_TEST 0U
#define RTC_TEN_SECOND_RELOAD 19999U
#define SENSOR_DATA_READY_TIMEOUT 500U

static USART_Handle_t g_usart2_handle = {0};
static SPI_Handle_t g_spi1_handle = {0};

static LIS3DSH_Data_t g_acceleration = {0};
static LIS3DSH_MgData_t g_acceleration_mg = {0};

static Orientation_t g_orientation = ORIENTATION_UNKNOWN;
static TiltDirection_t g_tiltDirection = TILT_LEVEL;

static GPIO_Handle_t g_activeMarkerPin = {0};


#if (ACTIVE_CURRENT_TEST == 1U)
static volatile uint32_t g_activeTestCounter = 0U;
#endif

/*static void Application_ShowTiltLED(TiltDirection_t tilt)
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
*/

static void Application_ActiveMarkerInit(void)
{
    g_activeMarkerPin.pGPIOx = GPIOE;
    g_activeMarkerPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    g_activeMarkerPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    g_activeMarkerPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    g_activeMarkerPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    g_activeMarkerPin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    g_activeMarkerPin.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&g_activeMarkerPin);
    GPIO_WriteToOutputPin(GPIOE, GPIO_PIN_NO_0, GPIO_PIN_RESET);
}

static void Application_ActiveMarkerOn(void)
{
    GPIO_WriteToOutputPin(GPIOE, GPIO_PIN_NO_0, GPIO_PIN_SET);
}

static void Application_ActiveMarkerOff(void)
{
    GPIO_WriteToOutputPin(GPIOE, GPIO_PIN_NO_0, GPIO_PIN_RESET);
}


static uint8_t Application_WaitForSensorDataReady(void)
{
    uint32_t timeout = SENSOR_DATA_READY_TIMEOUT;

    while ((LIS3DSH_IsDataReady() == 0U) && (timeout > 0U))
    {
        timeout--;
    }

    if (timeout == 0U)
    {
        return 0U;
    }

    return 1U;
}

static uint8_t Application_ReadFreshAccelerometerSample(LIS3DSH_Data_t *sample)
{
    LIS3DSH_Data_t discardedSample;

    if (sample == 0)
    {
        return 0U;
    }

    if (Application_WaitForSensorDataReady() == 0U)
    {
        return 0U;
    }

    LIS3DSH_ReadXYZ(&discardedSample);

    if (Application_WaitForSensorDataReady() == 0U)
    {
        return 0U;
    }

    LIS3DSH_ReadXYZ(sample);

    return 1U;
}

int main(void)
{
    char message[120];
    uint32_t currentWakeupCount;
    uint32_t previousWakeupCount = 0U;
    int messageLength;

    uint8_t startMessage[] = "PROJECT 2: RTC low-power sensor node\r\n";
    uint8_t sensorReadyMessage[] = "PASS: LIS3DSH initialized\r\n";
    uint8_t sensorFailureMessage[] = "FAIL: LIS3DSH initialization failed\r\n";
    uint8_t sensorStartFailureMessage[] = "FAIL: Could not start LIS3DSH measurement\r\n";
    uint8_t sensorPowerDownFailureMessage[] = "FAIL: LIS3DSH power-down failed\r\n";
    uint8_t clockFailureMessage[] = "FAIL: RTC clock initialization failed\r\n";
    uint8_t timerFailureMessage[] = "FAIL: RTC wake-up timer initialization failed\r\n";
    uint8_t timerReadyMessage[] = "PASS: RTC wake-up timer configured\r\n";
    uint8_t dataNotReadyMessage[] = "WARNING: Accelerometer data was not ready\r\n";

#if (ACTIVE_CURRENT_TEST == 1U)
    uint8_t activeTestMessage[] = "ACTIVE CURRENT TEST: CPU remains running\r\n";
#endif

    USART2_GPIO_Init();
    USART2_InitForTest(&g_usart2_handle);
    Console_Init(&g_usart2_handle);

    Board_LEDs_Init();
    //PE0 timing marker: HIGH while processing, LOW during Stop mode.
    Application_ActiveMarkerInit();
    Orientation_Init();
    MotionDetection_Init();

    Console_SendString(startMessage, sizeof(startMessage) - 1U);

    SPI1_GPIO_Init();
    SPI1_InitForAccelerometer(&g_spi1_handle);

    if (LIS3DSH_Init() == 0U)
    {
        Board_LED_On(BOARD_LED_RED);
        Console_SendString(sensorFailureMessage, sizeof(sensorFailureMessage) - 1U);

        while (1)
        {
        }
    }

    Console_SendString(sensorReadyMessage, sizeof(sensorReadyMessage) - 1U);

#if (ACTIVE_CURRENT_TEST == 1U)
    Console_SendString(activeTestMessage, sizeof(activeTestMessage) - 1U);
    Board_LEDs_Off();

    while (1)
    {
        g_activeTestCounter++;
    }
#endif

    if (LIS3DSH_EnterPowerDown() == 0U)
    {
        Board_LED_On(BOARD_LED_RED);
        Console_SendString(sensorPowerDownFailureMessage, sizeof(sensorPowerDownFailureMessage) - 1U);

        while (1)
        {
        }
    }

    LIS3DSH_ReadXYZ(&g_acceleration);

    if (RTC_ClockSource_Init() == 0U)
    {
        Board_LED_On(BOARD_LED_RED);
        Console_SendString(clockFailureMessage, sizeof(clockFailureMessage) - 1U);

        while (1)
        {
        }
    }

    if (RTC_WakeupTimer_Init(RTC_TEN_SECOND_RELOAD) == 0U)
    {
        Board_LED_On(BOARD_LED_RED);
        Console_SendString(timerFailureMessage, sizeof(timerFailureMessage) - 1U);

        while (1)
        {
        }
    }

    Console_SendString(timerReadyMessage, sizeof(timerReadyMessage) - 1U);

    while (1)
    {
        PWR_EnterStopMode();

        currentWakeupCount = RTC_GetWakeupCount();

        if (currentWakeupCount != previousWakeupCount)
        {
            previousWakeupCount = currentWakeupCount;
            Application_ActiveMarkerOn();



            if (LIS3DSH_SetOutputDataRate(100U) == 0U)
            {
                Board_LED_On(BOARD_LED_RED);
                Console_SendString(sensorStartFailureMessage, sizeof(sensorStartFailureMessage) - 1U);

                while (1)
                {
                }
            }

            if (Application_ReadFreshAccelerometerSample(&g_acceleration) == 0U)
            {
                if (LIS3DSH_EnterPowerDown() == 0U)
                {
                    Board_LED_On(BOARD_LED_RED);
                    Console_SendString(sensorPowerDownFailureMessage, sizeof(sensorPowerDownFailureMessage) - 1U);

                    while (1)
                    {
                    }
                }

                Board_LEDs_Off();
                Console_SendString(dataNotReadyMessage, sizeof(dataNotReadyMessage) - 1U);
            }
            else
            {
                LIS3DSH_ConvertRawToMg(&g_acceleration, &g_acceleration_mg);

                g_orientation = Orientation_Update(&g_acceleration_mg);

                if (g_orientation != ORIENTATION_MOVING)
                {
                    g_tiltDirection = MotionDetection_UpdateTilt(&g_acceleration_mg);
                }
                else
                {
                    g_tiltDirection = TILT_LEVEL;
                }

                if (LIS3DSH_EnterPowerDown() == 0U)
                {
                    Board_LED_On(BOARD_LED_RED);
                    Console_SendString(sensorPowerDownFailureMessage, sizeof(sensorPowerDownFailureMessage) - 1U);

                    while (1)
                    {
                    }
                }

                messageLength = snprintf(message, sizeof(message), "Wake=%lu, X=%ld mg, Y=%ld mg, Z=%ld mg, O=%s, T=%s\r\n", (unsigned long)currentWakeupCount, (long)g_acceleration_mg.x, (long)g_acceleration_mg.y, (long)g_acceleration_mg.z, Orientation_GetName(g_orientation), MotionDetection_GetTiltName(g_tiltDirection));

                if ((messageLength > 0) && ((uint32_t)messageLength < sizeof(message)))
                {
                    Console_SendString((uint8_t *)message, (uint32_t)messageLength);
                }

                Board_LEDs_Off();
            }
            Application_ActiveMarkerOff();
        }

    }
}
