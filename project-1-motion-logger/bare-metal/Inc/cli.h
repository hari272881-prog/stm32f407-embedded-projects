/*
 * cli.h
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_

#include "lis3dsh.h"
#include "orientation.h"
#include "motion_detection.h"
#include <stdint.h>

typedef struct
{
    const LIS3DSH_MgData_t *pAcceleration;
    const volatile Orientation_t *pOrientation;
    const volatile TiltDirection_t *pTiltDirection;
    const volatile uint32_t *pTapCount;
    const volatile uint32_t *pSampleCount;
    const volatile uint32_t *pFramesGenerated;
    const volatile uint32_t *pFramesDropped;
    volatile uint16_t *pSensorRateHz;
    volatile uint8_t *pTelemetryEnabled;
} CLI_Context_t;

void CLI_Init(const CLI_Context_t *context);
void CLI_ProcessByte(uint8_t receivedByte);

#endif
