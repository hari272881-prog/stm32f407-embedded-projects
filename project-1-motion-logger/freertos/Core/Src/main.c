/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensor_port.h"
#include "orientation.h"
#include "motion_detection.h"
#include "board.h"
#include "uart_async.h"
#include "cli.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_THREAD_FLAG_TAP (1U << 0)
#define APP_TAP_FLASH_MS 200U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for processingTask */
osThreadId_t processingTaskHandle;
const osThreadAttr_t processingTask_attributes = {
  .name = "processingTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for telemetryTask */
osThreadId_t telemetryTaskHandle;
const osThreadAttr_t telemetryTask_attributes = {
  .name = "telemetryTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for cliTask */
osThreadId_t cliTaskHandle;
const osThreadAttr_t cliTask_attributes = {
  .name = "cliTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensorQueue */
osMessageQueueId_t sensorQueueHandle;
const osMessageQueueAttr_t sensorQueue_attributes = {
  .name = "sensorQueue"
};
/* Definitions for uartMutex */
osMutexId_t uartMutexHandle;
const osMutexAttr_t uartMutex_attributes = {
  .name = "uartMutex"
};
/* Definitions for sensorMutex */
osMutexId_t sensorMutexHandle;
const osMutexAttr_t sensorMutex_attributes = {
  .name = "sensorMutex"
};
/* Definitions for tapSemaphore */
osSemaphoreId_t tapSemaphoreHandle;
const osSemaphoreAttr_t tapSemaphore_attributes = {
  .name = "tapSemaphore"
};
/* USER CODE BEGIN PV */

volatile uint8_t g_sensorInitStatus = 0U;
volatile uint8_t g_sensorId = 0U;
volatile uint32_t g_sensorTaskCount = 0U;
volatile int16_t g_sensorRawX = 0;
volatile int16_t g_sensorRawY = 0;
volatile int16_t g_sensorRawZ = 0;
volatile uint32_t g_sensorReadCount = 0U;

volatile uint32_t g_sensorQueueSendCount = 0U;
volatile uint32_t g_sensorQueueReceiveCount = 0U;
volatile uint32_t g_sensorQueueDropCount = 0U;

volatile int32_t g_sensorMgX = 0;
volatile int32_t g_sensorMgY = 0;
volatile int32_t g_sensorMgZ = 0;
volatile Orientation_t g_orientation = ORIENTATION_UNKNOWN;
volatile uint32_t g_processedSampleCount = 0U;

volatile TiltDirection_t g_tiltDirection = TILT_LEVEL;
volatile uint32_t g_tapIsrCount = 0U;

volatile uint8_t g_tapHardwareInitStatus = 0U;
volatile uint8_t g_lastTapInterruptSource = 0U;
volatile uint32_t g_tapCount = 0U;

static USART_Handle_t g_usart2Handle = {0};

const UART2_AsyncStats_t *g_uartStats = NULL;

volatile uint32_t g_telemetryFramesGenerated = 0U;
volatile uint32_t g_telemetryFramesDropped = 0U;

volatile uint8_t g_telemetryEnabled = 1U;
volatile uint16_t g_sensorRateHz = 100U;
volatile uint32_t g_rxBytesConsumed = 0U;
volatile LIS3DSH_MgData_t g_latestAccelerationMg = {0};

volatile uint32_t g_currentFreeHeapBytes = 0U;
volatile uint32_t g_minimumEverFreeHeapBytes = 0U;

volatile uint32_t g_defaultTaskStackFreeBytes = 0U;
volatile uint32_t g_sensorTaskStackFreeBytes = 0U;
volatile uint32_t g_processingTaskStackFreeBytes = 0U;
volatile uint32_t g_telemetryTaskStackFreeBytes = 0U;
volatile uint32_t g_cliTaskStackFreeBytes = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartDefaultTask(void *argument);
void StartSensorTask(void *argument);
void StartProcessingTask(void *argument);
void StartTelemetryTask(void *argument);
void StartCliTask(void *argument);

/* USER CODE BEGIN PFP */
static void Application_ShowTiltLED(TiltDirection_t tilt);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  USART2_GPIO_Init();
  USART2_InitForTest(&g_usart2Handle);
  USART2->BRR = (HAL_RCC_GetPCLK1Freq() + 57600U) / 115200U;
  UART2_AsyncInit();
  g_uartStats = UART2_AsyncGetStats();

  g_sensorInitStatus = SensorPort_Init();
  g_sensorId = SensorPort_ReadId();
  Orientation_Init();
  MotionDetection_Init();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of uartMutex */
  uartMutexHandle = osMutexNew(&uartMutex_attributes);

  /* creation of sensorMutex */
  sensorMutexHandle = osMutexNew(&sensorMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of tapSemaphore */
  tapSemaphoreHandle = osSemaphoreNew(1, 0, &tapSemaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensorQueue */
  sensorQueueHandle = osMessageQueueNew (8, sizeof(SensorPortRawData_t), &sensorQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of sensorTask */
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, &sensorTask_attributes);

  /* creation of processingTask */
  processingTaskHandle = osThreadNew(StartProcessingTask, NULL, &processingTask_attributes);

  /* creation of telemetryTask */
  telemetryTaskHandle = osThreadNew(StartTelemetryTask, NULL, &telemetryTask_attributes);

  /* creation of cliTask */
  cliTaskHandle = osThreadNew(StartCliTask, NULL, &cliTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2S3_WS_Pin */
  GPIO_InitStruct.Pin = I2S3_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(I2S3_WS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_12|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio_SCL_Pin Audio_SDA_Pin */
  GPIO_InitStruct.Pin = Audio_SCL_Pin|Audio_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT1_Pin */
  GPIO_InitStruct.Pin = MEMS_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void Application_ShowTiltLED(TiltDirection_t tilt)
{
    HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOD, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);

    if ((tilt & TILT_LEFT) != 0U)
    {
        HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
    }

    if ((tilt & TILT_RIGHT) != 0U)
    {
        HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    }

    if ((tilt & TILT_FORWARD) != 0U)
    {
        HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
    }

    if ((tilt & TILT_BACKWARD) != 0U)
    {
        HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ((GPIO_Pin == GPIO_PIN_0) && (tapSemaphoreHandle != NULL))
    {
        g_tapIsrCount++;
        (void)osSemaphoreRelease(tapSemaphoreHandle);
    }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* USER CODE BEGIN 5 */
    (void)argument;

    for (;;)
    {
        g_currentFreeHeapBytes = xPortGetFreeHeapSize();
        g_minimumEverFreeHeapBytes = xPortGetMinimumEverFreeHeapSize();

        g_defaultTaskStackFreeBytes = osThreadGetStackSpace(defaultTaskHandle);
        g_sensorTaskStackFreeBytes = osThreadGetStackSpace(sensorTaskHandle);
        g_processingTaskStackFreeBytes = osThreadGetStackSpace(processingTaskHandle);
        g_telemetryTaskStackFreeBytes = osThreadGetStackSpace(telemetryTaskHandle);
        g_cliTaskStackFreeBytes = osThreadGetStackSpace(cliTaskHandle);

        osDelay(1000U);
    }
    /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the sensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
    /* USER CODE BEGIN StartSensorTask */
    SensorPortRawData_t sample;
    uint32_t nextWakeTime;
    uint32_t sensorPeriodTicks;
    uint8_t sampleAvailable;

    (void)argument;

    HAL_NVIC_DisableIRQ(EXTI0_IRQn);

    if (osMutexAcquire(sensorMutexHandle, osWaitForever) == osOK)
    {
    	g_tapHardwareInitStatus = SensorPort_EnableDoubleTap(g_sensorRateHz);

    	osDelay(50U);

    	(void)SensorPort_ClearTapInterrupt();
    	(void)osMutexRelease(sensorMutexHandle);
    }

    __HAL_GPIO_EXTI_CLEAR_IT(MEMS_INT1_Pin);

    while (osSemaphoreAcquire(tapSemaphoreHandle, 0U) == osOK)
    {
    }

    g_tapIsrCount = 0U;
    g_tapCount = 0U;
    g_lastTapInterruptSource = 0U;

    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    nextWakeTime = osKernelGetTickCount();

    for (;;)
    {
        sampleAvailable = 0U;

        if (osMutexAcquire(sensorMutexHandle, osWaitForever) == osOK)
        {
            if ((g_sensorInitStatus != 0U) && (SensorPort_IsDataReady() != 0U))
            {
                SensorPort_ReadRaw(&sample);
                g_sensorReadCount++;
                sampleAvailable = 1U;
            }

            (void)osMutexRelease(sensorMutexHandle);
        }

        if (sampleAvailable != 0U)
        {
            if (osMessageQueuePut(sensorQueueHandle, &sample, 0U, 0U) == osOK)
            {
                g_sensorQueueSendCount++;
            }
            else
            {
                g_sensorQueueDropCount++;
            }
        }

        if (osSemaphoreAcquire(tapSemaphoreHandle, 0U) == osOK)
        {
            if (osMutexAcquire(sensorMutexHandle, osWaitForever) == osOK)
            {
                g_lastTapInterruptSource = SensorPort_ClearTapInterrupt();
                (void)osMutexRelease(sensorMutexHandle);
            }

            g_tapCount++;
            (void)osThreadFlagsSet(processingTaskHandle, APP_THREAD_FLAG_TAP);
        }

        sensorPeriodTicks = osKernelGetTickFreq() / (uint32_t)g_sensorRateHz;

        if (sensorPeriodTicks == 0U)
        {
            sensorPeriodTicks = 1U;
        }

        nextWakeTime += sensorPeriodTicks;
        osDelayUntil(nextWakeTime);
    }
    /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartProcessingTask */
/**
* @brief Function implementing the processingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProcessingTask */
void StartProcessingTask(void *argument)
{
  /* USER CODE BEGIN StartProcessingTask */
    SensorPortRawData_t receivedSample;
    LIS3DSH_Data_t rawData;
    LIS3DSH_MgData_t mgData;
    uint32_t receivedFlags;
    uint32_t tapFlashSamples = 0U;

    (void)argument;

    for (;;)
    {
        if (osMessageQueueGet(sensorQueueHandle, &receivedSample, NULL, osWaitForever) == osOK)
        {
            rawData.x = receivedSample.x;
            rawData.y = receivedSample.y;
            rawData.z = receivedSample.z;

            LIS3DSH_ConvertRawToMg(&rawData, &mgData);
            g_latestAccelerationMg = mgData;

            g_sensorRawX = rawData.x;
            g_sensorRawY = rawData.y;
            g_sensorRawZ = rawData.z;

            g_sensorMgX = mgData.x;
            g_sensorMgY = mgData.y;
            g_sensorMgZ = mgData.z;

            g_orientation = Orientation_Update(&mgData);
            g_tiltDirection = MotionDetection_UpdateTilt(&mgData);

            receivedFlags = osThreadFlagsWait(APP_THREAD_FLAG_TAP, osFlagsWaitAny, 0U);

            if (((receivedFlags & osFlagsError) == 0U) && ((receivedFlags & APP_THREAD_FLAG_TAP) != 0U))
            {
                tapFlashSamples = (((uint32_t)g_sensorRateHz * APP_TAP_FLASH_MS) + 999U) / 1000U;

                if (tapFlashSamples == 0U)
                {
                	tapFlashSamples = 1U;
                }
            }

            if (tapFlashSamples > 0U)
            {
                HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOD, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_SET);
                tapFlashSamples--;
            }
            else
            {
                Application_ShowTiltLED(g_tiltDirection);
            }

            g_sensorQueueReceiveCount++;
            g_processedSampleCount++;
        }
    }
  /* USER CODE END StartProcessingTask */
}

/* USER CODE BEGIN Header_StartTelemetryTask */
/**
* @brief Function implementing the telemetryTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTelemetryTask */
void StartTelemetryTask(void *argument)
{
  /* USER CODE BEGIN StartTelemetryTask */
    char message[160];
    int messageLength;
    uint8_t queuedSuccessfully;
    uint32_t nextWakeTime;
    uint32_t telemetryPeriodTicks;

    (void)argument;

    telemetryPeriodTicks = (osKernelGetTickFreq() * 100U) / 1000U;
    nextWakeTime = osKernelGetTickCount();

    for (;;)
    {
  if (g_telemetryEnabled != 0U)
    {
        messageLength = snprintf(message, sizeof(message), "X=%ld,Y=%ld,Z=%ld,O=%s,T=%s,Taps=%lu\r\n", (long)g_sensorMgX, (long)g_sensorMgY, (long)g_sensorMgZ, Orientation_GetName(g_orientation), MotionDetection_GetTiltName(g_tiltDirection), (unsigned long)g_tapCount);

        if ((messageLength > 0) && ((uint32_t)messageLength < sizeof(message)))
        {
            g_telemetryFramesGenerated++;

            if (osMutexAcquire(uartMutexHandle, osWaitForever) == osOK)
            {
                queuedSuccessfully = UART2_AsyncQueueTx((const uint8_t *)message, (uint16_t)messageLength);
                (void)osMutexRelease(uartMutexHandle);

                if (queuedSuccessfully == 0U)
                {
                    g_telemetryFramesDropped++;
                }
            }
            else
            {
                g_telemetryFramesDropped++;
            }
        }
        else
        {
            g_telemetryFramesDropped++;
        }
    }

    nextWakeTime += telemetryPeriodTicks;
    osDelayUntil(nextWakeTime);
    }
}
  /* USER CODE END StartTelemetryTask */

/* USER CODE BEGIN Header_StartCliTask */
/**
* @brief Function implementing the cliTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCliTask */
void StartCliTask(void *argument)
{
  /* USER CODE BEGIN StartCliTask */
    CLI_Context_t cliContext;
    uint8_t receivedByte;

    (void)argument;

    cliContext.pAcceleration = &g_latestAccelerationMg;
    cliContext.pOrientation = &g_orientation;
    cliContext.pTiltDirection = &g_tiltDirection;
    cliContext.pTapCount = &g_tapCount;
    cliContext.pSampleCount = &g_processedSampleCount;
    cliContext.pFramesGenerated = &g_telemetryFramesGenerated;
    cliContext.pFramesDropped = &g_telemetryFramesDropped;
    cliContext.pSensorRateHz = &g_sensorRateHz;
    cliContext.pTelemetryEnabled = &g_telemetryEnabled;

    CLI_Init(&cliContext);

    for (;;)
    {
        while (UART2_AsyncReadByte(&receivedByte) != 0U)
        {
            g_rxBytesConsumed++;
            CLI_ProcessByte(receivedByte);
        }

        osDelay(10U);
    }
  /* USER CODE END StartCliTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
