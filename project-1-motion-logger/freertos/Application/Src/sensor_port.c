#include "sensor_port.h"
#include "board.h"
#include "lis3dsh.h"

static SPI_Handle_t s_spi1Handle = {0};

uint8_t SensorPort_Init(void)
{
    SPI1_GPIO_Init();
    SPI1_InitForAccelerometer(&s_spi1Handle);

    return LIS3DSH_Init();
}

uint8_t SensorPort_ReadId(void)
{
    return LIS3DSH_ReadID();
}
uint8_t SensorPort_IsDataReady(void)
{
    return LIS3DSH_IsDataReady();
}

void SensorPort_ReadRaw(SensorPortRawData_t *data)
{
    LIS3DSH_Data_t rawData;

    if (data == 0)
    {
        return;
    }

    LIS3DSH_ReadXYZ(&rawData);

    data->x = rawData.x;
    data->y = rawData.y;
    data->z = rawData.z;
}


uint8_t SensorPort_EnableDoubleTap(uint16_t rateHz)
{
    return LIS3DSH_EnableDoubleTapInterrupt(rateHz);
}

uint8_t SensorPort_ClearTapInterrupt(void)
{
    return LIS3DSH_ClearTapInterrupt();
}
