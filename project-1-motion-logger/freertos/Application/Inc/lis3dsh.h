#ifndef LIS3DSH_H_
#define LIS3DSH_H_

#include <stdint.h>

#define LIS3DSH_EXPECTED_ID 0x3FU

#define LIS3DSH_CTRL_REG1             0x21U
#define LIS3DSH_CTRL_REG3             0x23U
#define LIS3DSH_CTRL_REG4             0x20U
#define LIS3DSH_CTRL_REG5             0x24U

#define LIS3DSH_SM1_PROGRAM_START     0x40U
#define LIS3DSH_TIM4_1                0x50U
#define LIS3DSH_TIM3_1                0x51U
#define LIS3DSH_TIM2_1_L              0x52U
#define LIS3DSH_TIM2_1_H              0x53U
#define LIS3DSH_TIM1_1_L              0x54U
#define LIS3DSH_TIM1_1_H              0x55U
#define LIS3DSH_THRS2_1               0x56U
#define LIS3DSH_THRS1_1               0x57U
#define LIS3DSH_MASK1_B               0x59U
#define LIS3DSH_MASK1_A               0x5AU
#define LIS3DSH_SETT1                 0x5BU
#define LIS3DSH_OUTS1                 0x5FU

/* LIS3DSH registers */
#define LIS3DSH_WHO_AM_I_REG      0x0FU
#define LIS3DSH_CTRL_REG4         0x20U
#define LIS3DSH_CTRL_REG5         0x24U
#define LIS3DSH_CTRL_REG6         0x25U
#define LIS3DSH_STATUS_REG        0x27U
#define LIS3DSH_OUT_X_L           0x28U

/* SPI protocol */
#define LIS3DSH_SPI_READ_BIT      0x80U
#define LIS3DSH_SPI_ADDRESS_MASK  0x7FU
#define LIS3DSH_SPI_DUMMY_BYTE    0x00U

/* Register bits */
#define LIS3DSH_CTRL6_ADD_INC     0x10U
#define LIS3DSH_STATUS_ZYXDA      0x08U

/* Sensor configuration */
#define LIS3DSH_CTRL_REG4_VALUE   0x6FU
#define LIS3DSH_CTRL_REG5_VALUE   0x00U

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t z;
} LIS3DSH_MgData_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} LIS3DSH_Data_t;

void LIS3DSH_ConvertRawToMg(const LIS3DSH_Data_t *rawData, LIS3DSH_MgData_t *mgData);

uint8_t LIS3DSH_Init(void);
uint8_t LIS3DSH_ReadID(void);
uint8_t LIS3DSH_ReadRegister(uint8_t address);
void LIS3DSH_WriteRegister(uint8_t address, uint8_t value);
void LIS3DSH_ReadXYZ(LIS3DSH_Data_t *data);
uint8_t LIS3DSH_IsDataReady(void);
uint8_t LIS3DSH_SetOutputDataRate(uint16_t rateHz);


uint8_t LIS3DSH_EnableDoubleTapInterrupt(uint16_t rateHz);
uint8_t LIS3DSH_SetDoubleTapOutputDataRate(uint16_t rateHz);
uint8_t LIS3DSH_ClearTapInterrupt(void);

#endif /* LIS3DSH_H_ */
