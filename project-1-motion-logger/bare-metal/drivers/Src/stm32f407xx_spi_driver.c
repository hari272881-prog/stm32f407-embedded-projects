/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Sep 26, 2026
 *      Author: R.S. Hari Naveen
 */


/*
 * Peripheral clock setup
 */
#include "stm32f407xx_spi_driver.h"

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(pSPIx == SPI1)
		{
			SPI1_PCLK_EN();
		}
		else if(pSPIx == SPI2)
		{
			SPI2_PCLK_EN();
		}
		else if(pSPIx == SPI3)
				{
					SPI3_PCLK_EN();
				}
		else if(pSPIx == SPI4)
				{
					SPI4_PCLK_EN();
				}

	}
	else
	{
		if(pSPIx == SPI1)
		{
			SPI1_PCLK_DI();
		}
		else if(pSPIx == SPI2)
		{
			SPI2_PCLK_DI();
		}
		else if(pSPIx == SPI3)
				{
					SPI3_PCLK_DI();
				}
		else if(pSPIx == SPI4)
				{
					SPI4_PCLK_DI();
				}
	}

}
/*
 * Init and De_init
 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	uint32_t tempreg=0;
	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

	//1.Configure the device mode
	tempreg|= pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR;

	//2. Configure the bus config
	if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
	{
		//BOIDIMODE cleared
		tempreg&= ~(1<<SPI_CR1_BIDIMODE);
	}
	else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
	{
		//BIDIMODE SET
		tempreg|= (1<<SPI_CR1_BIDIMODE);
	}
	else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		//BIDIMODE CLEARED
		tempreg&= ~(1<<SPI_CR1_BIDIMODE);
		//RXONLY bit must be set
		tempreg|= (1<<SPI_CR1_RXONLY);
	}
	//3. Configure the SPI serial clock speed (baud rate)
	tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR ;

	//4. configure the DFF
	tempreg |= pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;

	//5, configure the CPOL
	tempreg |= pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;

	//6.Configure the CPHA
	tempreg |= pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

	tempreg |= (uint32_t)pSPIHandle->SPIConfig.SPI_SSM<< SPI_CR1_SSM;

	pSPIHandle ->pSPIx->CR1 = tempreg ;
}
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
	if(pSPIx == SPI1)
	{
		SPI1_REG_RESET();

	}
	else if(pSPIx == SPI2)
	{
		SPI2_REG_RESET();

	}
	else if(pSPIx == SPI3)
	{
		SPI3_REG_RESET();
	}
	else if(pSPIx == SPI4)
	{
		SPI4_REG_RESET();
	}
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
	if(pSPIx->SR & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET ;
}
/*
 * Init and De_init
 */
/*********************************************************************
 * @fn            - SPI_SendData
 *
 * @brief         -
 *
 * @param[in]     -
 * @param[in]     -
 * @param[in]     -
 *
 * @return        - none
 *
 * @Note          - This is a blocking call
 *
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
	while(Len>0)
	{
		//1.Wait until the TXE is set (transmit buffer is ready)
		while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

		//2. Check the DFF bit
		if((pSPIx->CR1 &(1 << SPI_CR1_DFF)))
		{
			//16 bit DFF
			//1. Load the data in the data Register
			pSPIx->DR =   *((uint16_t*) pTxBuffer);

			//2. Reduce Len by 2 since we wrote 2 word data
			Len--;
			Len--;
			pTxBuffer ++; //Will be incremented by 2 due to typecast
			pTxBuffer ++;
		}
		else
		{
			//18 bit DFF
			//1. Load the data in the data Register
			pSPIx->DR =   *(pTxBuffer);

			//2. Reduce Len by 1
			Len--;
			pTxBuffer++;
		}
	}
}
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
	while(Len>0)
	{
		//1.Wait until the RXE is set (transmit buffer is ready)
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

		//2. Check the DFF bit
		if((pSPIx->CR1 &(1 << SPI_CR1_DFF)))
		{
			//16 bit DFF
			//1. Load the data from DR to Rxbuffer address
			*((uint16_t*)pRxBuffer) = pSPIx->DR ;

			//2. Reduce Len by 2 since we wrote 2 word data
			Len--;
			Len--;
			pRxBuffer ++; //Will be incremented by 2 due to typecast
			pRxBuffer ++;
		}
		else
		{
			//18 bit DFF
			//16 bit DFF
			//1. Load the data from DR to Rxbuffer address
			*(pRxBuffer) = pSPIx->DR ;

			//2. Reduce Len by 2 since we wrote 2 word data
			Len--;
			pRxBuffer++;
		}
	}

}

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
	uint8_t state = pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX)
	{
	//1. Save the Tx buffer address and Len information in some global variable
	pSPIHandle->pTxBuffer = pTxBuffer;
	pSPIHandle->TxLen= Len;

	//2. Mark the SPI state as busy in transmission so that
	// no other code can take over the same peripheral until the transmission is over
	pSPIHandle->TxState = SPI_BUSY_IN_TX;

	//3. Enable the TXEIE control bit to get the interrupt whenever the TXE flag is set in SR

	pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);

	//4. Data Transmission will be handled by the ISR code (will implement later)

	}

	return state;
}
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
	uint8_t state = pSPIHandle->RxState;
	if(state != SPI_BUSY_IN_RX)
	{
	//1. Save the Rx buffer address and Len information in some global variable
	pSPIHandle->pRxBuffer = pRxBuffer;
	pSPIHandle->RxLen= Len;

	//2. Mark the SPI state as busy in transmission so that
	// no other code can take over the same peripheral until the transmission is over
	pSPIHandle->RxState = SPI_BUSY_IN_RX;

	//3. Enable the RXNEIE control bit to get the interrupt whenever the RXE flag is set in SR

	pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);

	//4. Data Transmission will be handled by the ISR code (will implement later
	}
	return state;


}

/*
 * 	IRQ configuration and ISR Handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber , uint8_t EnorDi)
{

	if((EnorDi == ENABLE))
	{
		if(IRQNumber <= 31)
		{
			*NVIC_ISER0 = (1<< IRQNumber);
		}
		else if( IRQNumber >31 && IRQNumber < 64)
		{
			*NVIC_ISER1 = (1 << (IRQNumber%32));
		}
		else if( IRQNumber >=64 && IRQNumber < 96 )
		{
			*NVIC_ISER2 = (1 << (IRQNumber%64));
		}
		else if( IRQNumber >=96 && IRQNumber < 127 )
		{
			*NVIC_ISER3 = (1 << (IRQNumber%96));
		}
	}
	else
	{
		if(IRQNumber <= 31)
		{
			*NVIC_ICER0 = (1<< IRQNumber);
		}
		else if( IRQNumber >31 && IRQNumber < 64)
		{
			*NVIC_ICER1 = (1 << (IRQNumber%32));
		}
		else if( IRQNumber >=64 && IRQNumber < 96 )
		{
			*NVIC_ICER2 = (1 << (IRQNumber%64));
		}
		else if( IRQNumber >=96 && IRQNumber < 127 )
		{
			*NVIC_ICER3 = (1 << (IRQNumber%96));
		}
	}

}
void SPI_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority)
{
	//find the IPR register
	uint8_t iprx = IRQNumber/4;
	uint8_t iprx_selection = IRQNumber %4;

	*(NVIC_PR_BASE_ADDR + iprx ) |= (IRQPriority << ((8*iprx_selection) + (8 - NO_PR_BIT_IMPLEMENTED)));

}
void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
	uint8_t temp1, temp2;
	//first check for the TXE
	temp1= pHandle->pSPIx->SR & (1<< SPI_SR_TXE);
	temp2= pHandle->pSPIx->CR2 & (1<< SPI_CR2_TXEIE);

	if(temp1 &&temp2)
	{
		//handle TXE
		spi_txe_interrupt_handle(pHandle);
	}

	//Check for RXNE
	temp1= pHandle->pSPIx->SR & (1<< SPI_SR_RXNE);
	temp2= pHandle->pSPIx->CR2 & (1<< SPI_CR2_RXNEIE);

	if(temp1 &&temp2)
	{
		//handle TXE
		spi_rxne_interrupt_handle(pHandle);
	}

	//Check for ovr
	temp1= pHandle->pSPIx->SR & (1<< SPI_SR_OVR);
	temp2= pHandle->pSPIx->CR2 & (1<< SPI_CR2_ERRIE);

	if(temp1 &&temp2)
	{
		//handle TXE
		spi_ovr_err_interrupt_handle(pHandle);
	}


}
/*
 *  Other API
 */

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		pSPIx->CR1 |= (1<<SPI_CR1_SPE);
	}
	else
	{
		pSPIx->CR1 &= ~(1<<SPI_CR1_SPE);
	}
}

void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		pSPIx->CR1 |= (1<<SPI_CR1_SSI);
	}
	else
	{
		pSPIx->CR1 &= ~(1<<SPI_CR1_SSI);
	}
}

//Some helper function
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
		// Check the DFF bit
		if((pSPIHandle->pSPIx->CR1 &(1 << SPI_CR1_DFF)))
		{
			//16 bit DFF
			//1. load the data in the data Register
			uint16_t data = (uint16_t)pSPIHandle->pTxBuffer[0]
             | ((uint16_t)pSPIHandle->pTxBuffer[1] << 8);

			pSPIHandle->pSPIx->DR = data;

			//2. Reduce Len by 2 since we wrote 2 word data
			pSPIHandle->TxLen--;
			pSPIHandle->TxLen--;
			pSPIHandle-> pTxBuffer =pSPIHandle-> pTxBuffer+2; //Will be incremented by 2 due to typecast
		}
		else
		{
			//8 bit DFF
			//1. load the data in the data Register
			pSPIHandle->pSPIx->DR =   *(pSPIHandle->pTxBuffer);


			//2. Reduce Len by 2 since we wrote 2 word data
			pSPIHandle->TxLen--;
			pSPIHandle-> pTxBuffer ++; //Will be incremented by 2 due to typecast
		}
		if(! pSPIHandle-> TxLen)
		{
			//Txlen is zero , so close the SPI transmission and inform the application about it
			//Tx is over
			//This presents interrupts from setting the TXE flag
				// Disable the TXE interrupt; this does not clear the TXE flag.
			pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_TXEIE);

			pSPIHandle->pTxBuffer = NULL;
			pSPIHandle->TxLen = 0;
			pSPIHandle->TxState = SPI_READY;

			SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
		}
}
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
		//Check the DFF bit
		if((pSPIHandle->pSPIx->CR1 &(1 << SPI_CR1_DFF)))
		{
			//16 bit DFF
			//1. Read the data in the data Register
			uint16_t data = (uint16_t)pSPIHandle->pSPIx->DR;

			pSPIHandle->pRxBuffer[0] = (uint8_t)data;
			pSPIHandle->pRxBuffer[1] = (uint8_t)(data >> 8);


			//2. Reduce Len by 2 since we wrote 2 word data
			pSPIHandle->RxLen--;
			pSPIHandle->RxLen--;
			pSPIHandle-> pRxBuffer = pSPIHandle-> pRxBuffer+2; //Will be incremented by 2 due to typecast
		}
		else
		{
			//8 bit DFF
			//1. Read the data in the data Register
			*(pSPIHandle->pRxBuffer) = pSPIHandle->pSPIx->DR ;

			//2. Reduce Len by 1
			pSPIHandle->RxLen--;
			pSPIHandle->pRxBuffer++;
		}
		if(! pSPIHandle-> RxLen)
		{
			//Txlen is zero , so close the SPI transmission and inform the application about it
			//Tx is over
			//This presents interrupts from setting the TXE flag
				// Disable the TXE interrupt; this does not clear the TXE flag.
			pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_RXNEIE);

			pSPIHandle->pRxBuffer = NULL;
			pSPIHandle->RxLen = 0;
			pSPIHandle->RxState = SPI_READY;

			SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
		}

}
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp;
	//clear the ovr flag
	if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		temp= pSPIHandle-> pSPIx -> DR;
		temp= pSPIHandle-> pSPIx -> SR;
	}

	//inform application to clear flag on own
	SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
	(void)temp;
}

void SPI_Clear_OVRFlag(SPI_RegDef_t *pSPIx)
{
	uint8_t temp;

	temp= pSPIx -> DR;
	temp= pSPIx -> SR;
	(void)temp;
}
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
			pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_TXEIE);
			pSPIHandle->pTxBuffer = NULL;
			pSPIHandle->TxLen = 0;
			pSPIHandle->TxState = SPI_READY;

}
void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
			pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_RXNEIE);
			pSPIHandle->pRxBuffer = NULL;
			pSPIHandle->RxLen = 0;
			pSPIHandle->RxState = SPI_READY;
}
//Transmit and Recieve in same function for SPI1 in MEMS
uint8_t SPI_TransferByte(SPI_RegDef_t *pSPIx, uint8_t txByte)
{
    uint8_t rxByte;

    /* Wait until the SPI transmit register is available. */
    while ((pSPIx->SR & SPI_TXE_FLAG) == 0U)
    {
    }

    /*
     * Use an 8-bit access because SPI1 is configured
     * for an 8-bit data frame.
     */
    *(__vo uint8_t *)&pSPIx->DR = txByte;

    /* Wait until one byte has been received. */
    while ((pSPIx->SR & SPI_RXNE_FLAG) == 0U)
    {
    }

    rxByte = *(__vo uint8_t *)&pSPIx->DR;

    return rxByte;
}

__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle , uint8_t AppEv)
{
	//This is a weak implementation the application may override this function
}
