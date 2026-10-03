/*
 * lis3dsh.c
 *
 * Created on: Sep 30, 2026
 * Author: hari2
 */

#include "lis3dsh.h"
#include "board.h"
#include "stm32f407x_gpio_driver.h"
#include "stm32f407xx_spi_driver.h"


void LIS3DSH_ConvertRawToMg(const LIS3DSH_Data_t *rawData, LIS3DSH_MgData_t *mgData)
{
    if ((rawData == 0) || (mgData == 0))
    {
        return;
    }

    mgData->x = ((int32_t)rawData->x * 6) / 100;
    mgData->y = ((int32_t)rawData->y * 6) / 100;
    mgData->z = ((int32_t)rawData->z * 6) / 100;
}

static void LIS3DSH_Select(void)
{
    GPIO_WriteToOutputPin(ACCEL_CS_PORT, ACCEL_CS_PIN, GPIO_PIN_RESET);
}

static void LIS3DSH_Deselect(void)
{
    GPIO_WriteToOutputPin(ACCEL_CS_PORT, ACCEL_CS_PIN, GPIO_PIN_SET);
}

static void LIS3DSH_WaitUntilSPIIdle(void)
{
    while ((SPI1->SR & SPI_BUSY_FLAG) != 0U)
    {
    }
}

uint8_t LIS3DSH_ReadRegister(uint8_t address)
{
    uint8_t command;
    uint8_t value;

    command = (address & LIS3DSH_SPI_ADDRESS_MASK) | LIS3DSH_SPI_READ_BIT;

    LIS3DSH_Select();

    (void)SPI_TransferByte(SPI1, command);
    value = SPI_TransferByte(SPI1, LIS3DSH_SPI_DUMMY_BYTE);

    LIS3DSH_WaitUntilSPIIdle();
    LIS3DSH_Deselect();

    return value;
}

void LIS3DSH_WriteRegister(uint8_t address, uint8_t value)
{
    uint8_t command;

    command = address & LIS3DSH_SPI_ADDRESS_MASK;

    LIS3DSH_Select();

    (void)SPI_TransferByte(SPI1, command);
    (void)SPI_TransferByte(SPI1, value);

    LIS3DSH_WaitUntilSPIIdle();
    LIS3DSH_Deselect();
}

uint8_t LIS3DSH_ReadID(void)
{
    return LIS3DSH_ReadRegister(LIS3DSH_WHO_AM_I_REG);
}

uint8_t LIS3DSH_Init(void)
{
    uint8_t sensorID;
    uint8_t registerValue;
    uint8_t ctrl6Value;

    LIS3DSH_Deselect();

    sensorID = LIS3DSH_ReadID();

    if (sensorID != LIS3DSH_EXPECTED_ID)
    {
        return 0U;
    }

    ctrl6Value = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG6);
    ctrl6Value |= LIS3DSH_CTRL6_ADD_INC;
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG6, ctrl6Value);

    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG4, LIS3DSH_CTRL_REG4_VALUE);
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG5, LIS3DSH_CTRL_REG5_VALUE);

    registerValue = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG4);

    if (registerValue != LIS3DSH_CTRL_REG4_VALUE)
    {
        return 0U;
    }

    registerValue = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG6);

    if ((registerValue & LIS3DSH_CTRL6_ADD_INC) == 0U)
    {
        return 0U;
    }

    return 1U;
}

uint8_t LIS3DSH_IsDataReady(void)
{
    uint8_t status;

    status = LIS3DSH_ReadRegister(LIS3DSH_STATUS_REG);

    if ((status & LIS3DSH_STATUS_ZYXDA) != 0U)
    {
        return 1U;
    }

    return 0U;
}

void LIS3DSH_ReadXYZ(LIS3DSH_Data_t *data)
{
    uint8_t rawData[6];
    uint8_t command;
    uint8_t index;

    if (data == 0)
    {
        return;
    }

    command = (LIS3DSH_OUT_X_L & LIS3DSH_SPI_ADDRESS_MASK) | LIS3DSH_SPI_READ_BIT;

    LIS3DSH_Select();

    (void)SPI_TransferByte(SPI1, command);

    for (index = 0U; index < 6U; index++)
    {
        rawData[index] = SPI_TransferByte(SPI1, LIS3DSH_SPI_DUMMY_BYTE);
    }

    LIS3DSH_WaitUntilSPIIdle();
    LIS3DSH_Deselect();

    data->x = (int16_t)(((uint16_t)rawData[1] << 8U) | rawData[0]);
    data->y = (int16_t)(((uint16_t)rawData[3] << 8U) | rawData[2]);
    data->z = (int16_t)(((uint16_t)rawData[5] << 8U) | rawData[4]);
}

uint8_t LIS3DSH_SetOutputDataRate(uint16_t rateHz)
{
    uint8_t odrBits;
    uint8_t controlRegister;

    switch (rateHz)
    {
        case 25U:
            odrBits = 0x40U;
            break;

        case 50U:
            odrBits = 0x50U;
            break;

        case 100U:
            odrBits = 0x60U;
            break;

        case 400U:
            odrBits = 0x70U;
            break;

        default:
            return 0U;
    }

    /*
     * Preserve BDU and the X/Y/Z enable bits in the lower
     * four bits. Change only ODR[3:0] in bits 7:4.
     */
    controlRegister = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG4);
    controlRegister &= 0x0FU;
    controlRegister |= odrBits;

    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG4, controlRegister);

    /*
     * Read the register again to verify that the write worked.
     */
    controlRegister = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG4);

    if ((controlRegister & 0xF0U) != odrBits)
    {
        return 0U;
    }

    return 1U;
}

static uint16_t LIS3DSH_CalculateTimerCount(uint16_t durationMs,
                                            uint16_t rateHz)
{
    uint32_t timerCount;

    /*
     * Multiply by ODR and divide by 1000 to convert
     * milliseconds into sensor samples.
     *
     * Adding 500 performs rounding to the nearest count.
     */
    timerCount = ((uint32_t)durationMs * rateHz + 500U) / 1000U;

    /*
     * A zero timer would not provide a useful timing window.
     */
    if (timerCount == 0U)
    {
        timerCount = 1U;
    }

    return (uint16_t)timerCount;
}

static uint8_t LIS3DSH_WriteDoubleTapTiming(uint16_t rateHz)
{
    uint16_t timer1;
    uint16_t timer2;
    uint16_t timer3;
    uint16_t timer4;

    if ((rateHz != 25U) &&
        (rateHz != 50U) &&
        (rateHz != 100U) &&
        (rateHz != 400U))
    {
        return 0U;
    }
/*
 * More tolerant double-tap timing:
 *
 * TIM4: minimum interval after the first tap
 * TIM3: maximum duration allowed for a tap pulse
 * TIM2: complete double-tap recognition window
 * TIM1: required pre-silence interval
 */
timer4 = LIS3DSH_CalculateTimerCount(30U, rateHz);
timer3 = LIS3DSH_CalculateTimerCount(40U, rateHz);
timer2 = LIS3DSH_CalculateTimerCount(600U, rateHz);
timer1 = LIS3DSH_CalculateTimerCount(100U, rateHz);

    /*
     * TIM4 and TIM3 are 8-bit timers.
     */
    if ((timer4 > 255U) || (timer3 > 255U))
    {
        return 0U;
    }

    LIS3DSH_WriteRegister(LIS3DSH_TIM4_1,
                          (uint8_t)timer4);

    LIS3DSH_WriteRegister(LIS3DSH_TIM3_1,
                          (uint8_t)timer3);

    /*
     * TIM2 and TIM1 are 16-bit timers.
     */
    LIS3DSH_WriteRegister(LIS3DSH_TIM2_1_L,
                          (uint8_t)(timer2 & 0x00FFU));

    LIS3DSH_WriteRegister(LIS3DSH_TIM2_1_H,
                          (uint8_t)((timer2 >> 8U) & 0x00FFU));

    LIS3DSH_WriteRegister(LIS3DSH_TIM1_1_L,
                          (uint8_t)(timer1 & 0x00FFU));

    LIS3DSH_WriteRegister(LIS3DSH_TIM1_1_H,
                          (uint8_t)((timer1 >> 8U) & 0x00FFU));

    return 1U;
}

uint8_t LIS3DSH_EnableDoubleTapInterrupt(uint16_t rateHz)
{
    static const uint8_t doubleTapProgram[] =
    {
        0x51U,
        0x51U,
        0x06U,
        0x38U,
        0x04U,
        0x91U,
        0x26U,
        0x38U,
        0x04U,
        0x91U,
        0x11U
    };

    uint8_t registerValue;
    uint32_t index;

    /*
     * Only these ODR values are currently supported.
     */
    if ((rateHz != 25U) &&
        (rateHz != 50U) &&
        (rateHz != 100U) &&
        (rateHz != 400U))
    {
        return 0U;
    }

    /*
     * Disable SM1 and INT1 while configuring the sensor.
     */
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG1, 0x00U);
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG3, 0x00U);

    /*
     * Change the actual LIS3DSH output data rate.
     */
    if (LIS3DSH_SetOutputDataRate(rateHz) == 0U)
    {
        return 0U;
    }

    /*
     * ±2 g, self-test disabled, 4-wire SPI.
     */
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG5, 0x00U);

    /*
     * Write timer counts appropriate for the selected ODR.
     */
    if (LIS3DSH_WriteDoubleTapTiming(rateHz) == 0U)
    {
        return 0U;
    }

    /*
     * Thresholds remain the same because the full-scale
     * setting remains ±2 g.
     */
    LIS3DSH_WriteRegister(LIS3DSH_THRS2_1, 0x44U);
    LIS3DSH_WriteRegister(LIS3DSH_THRS1_1, 0x55U);

    /*
     * Enable positive and negative detection on X, Y and Z:
     *
     * Bit 7: positive X
     * Bit 6: negative X
     * Bit 5: positive Y
     * Bit 4: negative Y
     * Bit 3: positive Z
     * Bit 2: negative Z
     *
     * Bits 1 and 0 are vector-sign masks and remain disabled.
     */
    LIS3DSH_WriteRegister(LIS3DSH_MASK1_B, 0xFCU);
    LIS3DSH_WriteRegister(LIS3DSH_MASK1_A, 0xFCU);

   /*
    * SITR = 1:
    * CONT and STOP instructions generate an interrupt and
    * perform the corresponding output action.
    */
    LIS3DSH_WriteRegister(LIS3DSH_SETT1, 0x01U);

    /*
     * Clear all sixteen State Machine 1 program slots.
     */
    for (index = 0U; index < 16U; index++)
    {
        LIS3DSH_WriteRegister(
            (uint8_t)(LIS3DSH_SM1_PROGRAM_START + index),
            0x00U);
    }

    /*
     * Load the eleven-state double-tap program.
     */
    for (index = 0U;
         index < (uint32_t)sizeof(doubleTapProgram);
         index++)
    {
        LIS3DSH_WriteRegister(
            (uint8_t)(LIS3DSH_SM1_PROGRAM_START + index),
            doubleTapProgram[index]);
    }

    /*
     * Release any old latched interrupt.
     */
    (void)LIS3DSH_ReadRegister(LIS3DSH_OUTS1);

    /*
     * Active-high, latched interrupt on INT1.
     */
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG3, 0x48U);

    /*
     * Enable State Machine 1 and route it to INT1.
     */
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG1, 0x01U);

    /*
     * Verify the readable control registers.
     */
    registerValue = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG1);

    if ((registerValue & 0x01U) == 0U)
    {
        return 0U;
    }

    registerValue = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG3);

    if ((registerValue & 0x48U) != 0x48U)
    {
        return 0U;
    }

    return 1U;
}
uint8_t LIS3DSH_SetDoubleTapOutputDataRate(uint16_t rateHz)
{
    uint8_t registerValue;

    if ((rateHz != 25U) &&
        (rateHz != 50U) &&
        (rateHz != 100U) &&
        (rateHz != 400U))
    {
        return 0U;
    }

    /*
     * Temporarily disable State Machine 1 and its interrupt
     * output while changing its timing.
     */
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG1, 0x00U);
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG3, 0x00U);

    /*
     * Change the LIS3DSH ODR.
     */
    if (LIS3DSH_SetOutputDataRate(rateHz) == 0U)
    {
        return 0U;
    }

    /*
     * Scale the hardware tap timing for the new ODR.
     */
    if (LIS3DSH_WriteDoubleTapTiming(rateHz) == 0U)
    {
        return 0U;
    }

    /*
     * Release an old latched State Machine 1 interrupt.
     */
    (void)LIS3DSH_ReadRegister(LIS3DSH_OUTS1);

    /*
     * Re-enable INT1 and State Machine 1.
     */
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG3, 0x48U);
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG1, 0x01U);

    registerValue = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG1);

    if ((registerValue & 0x01U) == 0U)
    {
        return 0U;
    }

    registerValue = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG3);

    if ((registerValue & 0x48U) != 0x48U)
    {
        return 0U;
    }

    return 1U;
}
uint8_t LIS3DSH_ClearTapInterrupt(void)
{
    /*
     * The INT1 output is configured as latched.
     * Reading OUTS1 releases the interrupt output.
     */
    return LIS3DSH_ReadRegister(LIS3DSH_OUTS1);
}


uint8_t LIS3DSH_EnterPowerDown(void)
{
    uint8_t controlRegister;

    controlRegister = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG4);
    controlRegister &= 0x0FU;
    LIS3DSH_WriteRegister(LIS3DSH_CTRL_REG4, controlRegister);

    controlRegister = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG4);

    if ((controlRegister & 0xF0U) != 0U)
    {
        return 0U;
    }

    return 1U;
}
