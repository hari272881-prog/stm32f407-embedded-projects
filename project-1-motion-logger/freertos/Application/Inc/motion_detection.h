/*
 * motion_detection.h
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#ifndef MOTION_DETECTION_H_
#define MOTION_DETECTION_H_


#include "lis3dsh.h"

typedef enum
{
    TILT_LEVEL = 0U,
    TILT_LEFT = (1U << 0),
    TILT_RIGHT = (1U << 1),
    TILT_FORWARD = (1U << 2),
    TILT_BACKWARD = (1U << 3),
    TILT_LEFT_FORWARD = TILT_LEFT | TILT_FORWARD,
    TILT_LEFT_BACKWARD = TILT_LEFT | TILT_BACKWARD,
    TILT_RIGHT_FORWARD = TILT_RIGHT | TILT_FORWARD,
    TILT_RIGHT_BACKWARD = TILT_RIGHT | TILT_BACKWARD
} TiltDirection_t;

void MotionDetection_Init(void);
TiltDirection_t MotionDetection_UpdateTilt(const LIS3DSH_MgData_t *data);
uint8_t MotionDetection_CheckTap(const LIS3DSH_MgData_t *data);
const char *MotionDetection_GetTiltName(TiltDirection_t tilt);

#endif /* MOTION_DETECTION_H_ */
