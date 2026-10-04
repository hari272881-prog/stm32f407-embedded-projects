/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
volatile uint32_t g_stackOverflowDetected = 0U;
volatile uint32_t g_mallocFailureDetected = 0U;
const char * volatile g_stackOverflowTaskName = 0;

void vApplicationStackOverflowHook(TaskHandle_t taskHandle, char *taskName)
{
    (void)taskHandle;

    g_stackOverflowDetected = 1U;
    g_stackOverflowTaskName = taskName;

    taskDISABLE_INTERRUPTS();

    for (;;)
    {
    }
}

void vApplicationMallocFailedHook(void)
{
    g_mallocFailureDetected = 1U;

    taskDISABLE_INTERRUPTS();

    for (;;)
    {
    }
}

/* USER CODE END Application */

