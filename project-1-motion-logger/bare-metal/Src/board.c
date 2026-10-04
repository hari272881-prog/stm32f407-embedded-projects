/*
 * board.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#include "board.h"
#include "lis3dsh.h"
/* USART functions */
void USART2_GPIO_Init(void)
{

    GPIO_Handle_t gpio = {0};

    GPIO_PeriClockControl(GPIOA, ENABLE);

    gpio.pGPIOx = GPIOA;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 7U;
    gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    /* PA2: USART2_TX */
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    GPIO_Init(&gpio);

    /* PA3: USART2_RX */
     gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PU; //UART RX line should normally be high when nothing is transmitting
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
    GPIO_Init(&gpio);

}
void USART2_InitForTest(USART_Handle_t *p_usart2_handle)
{
	p_usart2_handle->pUSARTx = USART2;

    p_usart2_handle->USART_Config.USART_Mode = USART_MODE_TXRX;

    p_usart2_handle->USART_Config.USART_Baud = USART_STD_BAUD_115200;

    p_usart2_handle->USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;

    p_usart2_handle->USART_Config.USART_WordLength = USART_WORDLEN_8BITS;

    p_usart2_handle->USART_Config.USART_ParityControl = USART_PARITY_DISABLE;

    p_usart2_handle->USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_Init(p_usart2_handle);
    USART_PeripheralControl(USART2, ENABLE);

}

/* SPI and accelerometer functions */
void SPI1_GPIO_Init(void)
{
	/*PA5 = SPI1_SCK
	PA6 = SPI1_MISO
	PA7 = SPI1_MOSI
	PE3 = Accelerometer CS*/
	GPIO_Handle_t spi ={0};
	GPIO_Handle_t ChipSelect={0};
	GPIO_PeriClockControl(GPIOA, ENABLE);
	spi.pGPIOx= GPIOA;
	spi.GPIO_PinConfig.GPIO_PinAltFunMode = 5U;
	spi.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	spi.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	spi.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	spi.GPIO_PinConfig.GPIO_PinPuPdControl= GPIO_NO_PUPD;

	spi.GPIO_PinConfig.GPIO_PinNumber= GPIO_PIN_NO_5;
	GPIO_Init(&spi);

	spi.GPIO_PinConfig.GPIO_PinNumber= GPIO_PIN_NO_6;
	GPIO_Init(&spi);

	spi.GPIO_PinConfig.GPIO_PinNumber= GPIO_PIN_NO_7;
	GPIO_Init(&spi);

	GPIO_PeriClockControl(GPIOE, ENABLE);

	ChipSelect.pGPIOx = GPIOE;
	ChipSelect.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	ChipSelect.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PU;
	ChipSelect.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	ChipSelect.GPIO_PinConfig.GPIO_PinOPType= GPIO_OP_TYPE_PP;
	ChipSelect.GPIO_PinConfig.GPIO_PinSpeed= GPIO_SPEED_FAST;
	GPIO_Init(&ChipSelect);

	/* CS is active-low, so idle state must be high. */
	GPIO_WriteToOutputPin(ACCEL_CS_PORT,ACCEL_CS_PIN,GPIO_PIN_SET);


}
void SPI1_InitForAccelerometer(SPI_Handle_t *p_spi1_handle)
{
	p_spi1_handle->pSPIx = SPI1;
	p_spi1_handle->SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	p_spi1_handle->SPIConfig.SPI_CPHA = SPI_CPHA_HIGH;
	p_spi1_handle->SPIConfig.SPI_CPOL = SPI_CPOL_HIGH;
	p_spi1_handle->SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	p_spi1_handle->SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	p_spi1_handle->SPIConfig.SPI_SSM = SPI_SSM_EN;
	p_spi1_handle->SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV256;

	SPI_Init(p_spi1_handle);
	/*
	 * Keep the internal NSS signal high when software
	 * slave management is enabled, preventing mode fault.
	 */

	SPI_SSIConfig(SPI1, ENABLE);

	SPI_PeripheralControl(SPI1, ENABLE);

}

void Board_LEDs_Init(void)
{
    GPIO_Handle_t ledGPIO = {0};

    GPIO_PeriClockControl(BOARD_LED_PORT, ENABLE);

    ledGPIO.pGPIOx = BOARD_LED_PORT;
    ledGPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    ledGPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    ledGPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    ledGPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    ledGPIO.GPIO_PinConfig.GPIO_PinNumber = BOARD_LED_GREEN;
    GPIO_Init(&ledGPIO);

    ledGPIO.GPIO_PinConfig.GPIO_PinNumber = BOARD_LED_ORANGE;
    GPIO_Init(&ledGPIO);

    ledGPIO.GPIO_PinConfig.GPIO_PinNumber = BOARD_LED_RED;
    GPIO_Init(&ledGPIO);

    ledGPIO.GPIO_PinConfig.GPIO_PinNumber = BOARD_LED_BLUE;
    GPIO_Init(&ledGPIO);

    Board_LEDs_Off();
}

void Board_LEDs_Off(void)
{
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_GREEN, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_ORANGE, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_RED, GPIO_PIN_RESET);
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_BLUE, GPIO_PIN_RESET);
}

void Board_LEDs_On(void)
{
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_GREEN, GPIO_PIN_SET);
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_ORANGE, GPIO_PIN_SET);
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_RED, GPIO_PIN_SET);
    GPIO_WriteToOutputPin(BOARD_LED_PORT, BOARD_LED_BLUE, GPIO_PIN_SET);
}

void Board_LED_On(uint8_t ledPin)
{
    GPIO_WriteToOutputPin(BOARD_LED_PORT, ledPin, GPIO_PIN_SET);
}
