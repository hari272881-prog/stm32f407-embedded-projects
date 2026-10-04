/*
 * orientation.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */


#include "orientation.h"

#define ORIENTATION_FILTER_DIVISOR      8
#define ORIENTATION_AXIS_MINIMUM_MG     600
#define ORIENTATION_DOMINANCE_MARGIN_MG 100
#define ORIENTATION_MIN_GRAVITY_MG      700
#define ORIENTATION_MAX_GRAVITY_MG      1300

static LIS3DSH_MgData_t g_filteredData;
static uint8_t g_filterInitialized;

static int32_t Orientation_AbsoluteValue(int32_t value)
{
    return (value < 0) ? -value : value;
}

void Orientation_Init(void)
{
    g_filteredData.x = 0;
    g_filteredData.y = 0;
    g_filteredData.z = 0;
    g_filterInitialized = 0U;
}

static void Orientation_Filter(const LIS3DSH_MgData_t *newSample)
{
    if (g_filterInitialized == 0U)
    {
        g_filteredData = *newSample;
        g_filterInitialized = 1U;
        return;
    }

    g_filteredData.x += (newSample->x - g_filteredData.x) / ORIENTATION_FILTER_DIVISOR;
    g_filteredData.y += (newSample->y - g_filteredData.y) / ORIENTATION_FILTER_DIVISOR;
    g_filteredData.z += (newSample->z - g_filteredData.z) / ORIENTATION_FILTER_DIVISOR;
}

Orientation_t Orientation_Update(const LIS3DSH_MgData_t *newSample)
{
    int32_t absoluteX;
    int32_t absoluteY;
    int32_t absoluteZ;
    int64_t magnitudeSquared;
    int64_t minimumMagnitudeSquared;
    int64_t maximumMagnitudeSquared;

    if (newSample == 0)
    {
        return ORIENTATION_UNKNOWN;
    }

    Orientation_Filter(newSample);

    absoluteX = Orientation_AbsoluteValue(g_filteredData.x);
    absoluteY = Orientation_AbsoluteValue(g_filteredData.y);
    absoluteZ = Orientation_AbsoluteValue(g_filteredData.z);

    magnitudeSquared = ((int64_t)g_filteredData.x * g_filteredData.x) +
                       ((int64_t)g_filteredData.y * g_filteredData.y) +
                       ((int64_t)g_filteredData.z * g_filteredData.z);

    minimumMagnitudeSquared = (int64_t)ORIENTATION_MIN_GRAVITY_MG * ORIENTATION_MIN_GRAVITY_MG;
    maximumMagnitudeSquared = (int64_t)ORIENTATION_MAX_GRAVITY_MG * ORIENTATION_MAX_GRAVITY_MG;

    if ((magnitudeSquared < minimumMagnitudeSquared) || (magnitudeSquared > maximumMagnitudeSquared))
    {
        return ORIENTATION_MOVING;
    }

    if ((absoluteX >= ORIENTATION_AXIS_MINIMUM_MG) &&
        (absoluteX >= absoluteY + ORIENTATION_DOMINANCE_MARGIN_MG) &&
        (absoluteX >= absoluteZ + ORIENTATION_DOMINANCE_MARGIN_MG))
    {
        return (g_filteredData.x < 0) ? ORIENTATION_LANDSCAPE_LEFT : ORIENTATION_LANDSCAPE_RIGHT;
    }

    if ((absoluteY >= ORIENTATION_AXIS_MINIMUM_MG) &&
        (absoluteY >= absoluteX + ORIENTATION_DOMINANCE_MARGIN_MG) &&
        (absoluteY >= absoluteZ + ORIENTATION_DOMINANCE_MARGIN_MG))
    {
        return (g_filteredData.y > 0) ? ORIENTATION_REVERSE_PORTRAIT : ORIENTATION_PORTRAIT;
    }

    if ((absoluteZ >= ORIENTATION_AXIS_MINIMUM_MG) &&
        (absoluteZ >= absoluteX + ORIENTATION_DOMINANCE_MARGIN_MG) &&
        (absoluteZ >= absoluteY + ORIENTATION_DOMINANCE_MARGIN_MG))
    {
        return (g_filteredData.z > 0) ? ORIENTATION_FACE_UP : ORIENTATION_FACE_DOWN;
    }

    return ORIENTATION_UNKNOWN;
}

const char *Orientation_GetName(Orientation_t orientation)
{
    switch (orientation)
    {
        case ORIENTATION_FACE_UP:
            return "FACE UP";

        case ORIENTATION_FACE_DOWN:
            return "FACE DOWN";

        case ORIENTATION_LANDSCAPE_LEFT:
            return "LANDSCAPE LEFT";

        case ORIENTATION_LANDSCAPE_RIGHT:
            return "LANDSCAPE RIGHT";

        case ORIENTATION_PORTRAIT:
            return "PORTRAIT";

        case ORIENTATION_REVERSE_PORTRAIT:
            return "REVERSE PORTRAIT";

        case ORIENTATION_MOVING:
            return "MOVING";

        default:
            return "UNKNOWN";
    }
}
