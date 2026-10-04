/*
 * orientation.h
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#ifndef ORIENTATION_H_
#define ORIENTATION_H_

#include "lis3dsh.h"

typedef enum
{
    ORIENTATION_UNKNOWN = 0,
    ORIENTATION_MOVING,
    ORIENTATION_FACE_UP,
    ORIENTATION_FACE_DOWN,
    ORIENTATION_LANDSCAPE_LEFT,
    ORIENTATION_LANDSCAPE_RIGHT,
    ORIENTATION_PORTRAIT,
    ORIENTATION_REVERSE_PORTRAIT
} Orientation_t;

void Orientation_Init(void);
Orientation_t Orientation_Update(const LIS3DSH_MgData_t *newSample);
const char *Orientation_GetName(Orientation_t orientation);

#endif /* ORIENTATION_H_ */
