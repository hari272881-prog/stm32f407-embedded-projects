/*
 * motion_detection.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#include "motion_detection.h"

#define TILT_ACTIVATE_THRESHOLD_MG  350
#define TILT_RELEASE_THRESHOLD_MG   250
#define TAP_DELTA_THRESHOLD_MG      600
#define TAP_COOLDOWN_SAMPLES        20U

static LIS3DSH_MgData_t g_previousSample;
static uint8_t g_previousSampleValid;
static uint32_t g_tapCooldown;
static TiltDirection_t g_currentTilt;

static int32_t MotionDetection_AbsoluteValue(int32_t value)
{
    return (value < 0) ? -value : value;
}

void MotionDetection_Init(void)
{
    g_previousSample.x = 0;
    g_previousSample.y = 0;
    g_previousSample.z = 0;

    g_previousSampleValid = 0U;
    g_tapCooldown = 0U;
    g_currentTilt = TILT_LEVEL;
}

TiltDirection_t MotionDetection_UpdateTilt(const LIS3DSH_MgData_t *data)
{
    TiltDirection_t newTilt = TILT_LEVEL;

    if (data == 0)
    {
        return TILT_LEVEL;
    }

    if (data->x <= -TILT_ACTIVATE_THRESHOLD_MG)
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_LEFT);
    }
    else if (data->x >= TILT_ACTIVATE_THRESHOLD_MG)
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_RIGHT);
    }
    else if (((g_currentTilt & TILT_LEFT) != 0U) && (data->x <= -TILT_RELEASE_THRESHOLD_MG))
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_LEFT);
    }
    else if (((g_currentTilt & TILT_RIGHT) != 0U) && (data->x >= TILT_RELEASE_THRESHOLD_MG))
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_RIGHT);
    }

    if (data->y >= TILT_ACTIVATE_THRESHOLD_MG)
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_FORWARD);
    }
    else if (data->y <= -TILT_ACTIVATE_THRESHOLD_MG)
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_BACKWARD);
    }
    else if (((g_currentTilt & TILT_FORWARD) != 0U) && (data->y >= TILT_RELEASE_THRESHOLD_MG))
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_FORWARD);
    }
    else if (((g_currentTilt & TILT_BACKWARD) != 0U) && (data->y <= -TILT_RELEASE_THRESHOLD_MG))
    {
        newTilt = (TiltDirection_t)(newTilt | TILT_BACKWARD);
    }

    g_currentTilt = newTilt;

    return g_currentTilt;
}

uint8_t MotionDetection_CheckTap(const LIS3DSH_MgData_t *data)
{
    int32_t deltaX;
    int32_t deltaY;
    int32_t deltaZ;
    int32_t maximumDelta;

    if (data == 0)
    {
        return 0U;
    }

    if (g_tapCooldown > 0U)
    {
        g_tapCooldown--;
    }

    if (g_previousSampleValid == 0U)
    {
        g_previousSample = *data;
        g_previousSampleValid = 1U;
        return 0U;
    }

    deltaX = MotionDetection_AbsoluteValue(data->x - g_previousSample.x);
    deltaY = MotionDetection_AbsoluteValue(data->y - g_previousSample.y);
    deltaZ = MotionDetection_AbsoluteValue(data->z - g_previousSample.z);

    g_previousSample = *data;

    maximumDelta = deltaX;

    if (deltaY > maximumDelta)
    {
        maximumDelta = deltaY;
    }

    if (deltaZ > maximumDelta)
    {
        maximumDelta = deltaZ;
    }

    if ((maximumDelta >= TAP_DELTA_THRESHOLD_MG) && (g_tapCooldown == 0U))
    {
        g_tapCooldown = TAP_COOLDOWN_SAMPLES;
        return 1U;
    }

    return 0U;
}

const char *MotionDetection_GetTiltName(TiltDirection_t tilt)
{
    switch (tilt)
    {
        case TILT_LEVEL:
            return "LEVEL";

        case TILT_LEFT:
            return "LEFT";

        case TILT_RIGHT:
            return "RIGHT";

        case TILT_FORWARD:
            return "FORWARD";

        case TILT_BACKWARD:
            return "BACKWARD";

        case TILT_LEFT_FORWARD:
            return "LEFT + FORWARD";

        case TILT_LEFT_BACKWARD:
            return "LEFT + BACKWARD";

        case TILT_RIGHT_FORWARD:
            return "RIGHT + FORWARD";

        case TILT_RIGHT_BACKWARD:
            return "RIGHT + BACKWARD";

        default:
            return "UNKNOWN";
    }
}
