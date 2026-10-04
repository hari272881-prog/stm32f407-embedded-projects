/*
 * application.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */


#include "application.h"
#include "lis3dsh.h"
#include "console.h"
#include "board.h"

static LIS3DSH_Data_t s_acceleration;

void App_Init(void)
{
    /* Initialize application state. */
}

void App_Run(void)
{
    if (LIS3DSH_IsDataReady() != 0U)
    {
        LIS3DSH_ReadXYZ(&s_acceleration);

        /*
         * Filtering, tilt detection and LED control
         * will be added here.
         */
    }
}
