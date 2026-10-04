#ifndef SENSOR_PORT_H_
#define SENSOR_PORT_H_

#include <stdint.h>

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} SensorPortRawData_t;

uint8_t SensorPort_Init(void);
uint8_t SensorPort_ReadId(void);
uint8_t SensorPort_IsDataReady(void);
void SensorPort_ReadRaw(SensorPortRawData_t *data);
uint8_t SensorPort_EnableDoubleTap(uint16_t rateHz);
uint8_t SensorPort_ClearTapInterrupt(void);
#endif
