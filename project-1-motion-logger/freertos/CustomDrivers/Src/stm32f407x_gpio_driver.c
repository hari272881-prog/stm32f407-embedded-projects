/*
 * stm32f407x_gpio_driver.c
 *
 *  Created on: Sep 23, 2026
 *      Author: R.S. Hari Naveen
 */

#include "stm32f407x_gpio_driver.h"

/*
 * Init and De_init
 */
/*********************************************************************
 * @fn            - GPIO_Init
 *
 * @brief         - This function Initialises the GPIO port
 *
 * @param[in]     - base address of the gpio peripheral
 * @param[in]     - ENABLE or DISABLE macros
 * @param[in]     -
 *
 * @return        - none
 *
 * @Note          - none
 *
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{

	GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);
	uint32_t temp=0; //temp register
	//1.Configure the mode of GPIO pin

	if(pGPIOHandle -> GPIO_PinConfig.GPIO_PinMode <= CUSTOM_GPIO_MODE_ANALOG)
	{
		// the non interrupt mode
		temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
		pGPIOHandle->pGPIOx->MODER &= ~(0x3 << 2*pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		pGPIOHandle->pGPIOx->MODER |= temp;
		temp=0;
	}
	else
	{
		//interrupt mode
		if(pGPIOHandle -> GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
		{
			// Configure the FTSR
			CUSTOM_EXTI->FTSR |= (1<< pGPIOHandle ->GPIO_PinConfig.GPIO_PinNumber);
			//Clear the corresponding RTSR bit
			CUSTOM_EXTI->RTSR &= ~(1<<pGPIOHandle -> GPIO_PinConfig.GPIO_PinNumber);
		}
		else if(pGPIOHandle -> GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
		{
			// Configure the RTSR
			CUSTOM_EXTI->RTSR |= (1<< pGPIOHandle ->GPIO_PinConfig.GPIO_PinNumber);
			//Clear the corresponding RTSR bit
			CUSTOM_EXTI->FTSR &= ~(1<< pGPIOHandle ->GPIO_PinConfig.GPIO_PinNumber);
		}
		else if(pGPIOHandle -> GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT)
		{
			// Configure the FTSR and RTSR
			CUSTOM_EXTI->FTSR |= (1<< pGPIOHandle ->GPIO_PinConfig.GPIO_PinNumber);
			CUSTOM_EXTI->RTSR |= (1<< pGPIOHandle ->GPIO_PinConfig.GPIO_PinNumber);
		}

		//configure the GPIO port selection in the SYSCFG_EXTICR
		uint8_t temp1 = pGPIOHandle -> GPIO_PinConfig.GPIO_PinNumber/4;
		uint8_t temp2 = pGPIOHandle -> GPIO_PinConfig.GPIO_PinNumber%4;
		uint8_t portcode = GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);
		SYSCFG_PCLK_EN();
		CUSTOM_SYSCFG->EXTICR[temp1]= portcode << (temp2 *4);

		//enable the CUSTOM_EXTI interrupt delivery using IMR
		CUSTOM_EXTI->IMR |= 1<< pGPIOHandle -> GPIO_PinConfig.GPIO_PinNumber ;
	}

	//2. Config the speed
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3 << 2*pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandle->pGPIOx->OSPEEDR |= temp;
	temp=0;

	//3. config the pupd settings
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->PUPDR &= ~(0x3 << 2*pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandle->pGPIOx->PUPDR |= temp;
	temp=0;

	//4. configure the optype
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->OTYPER &= ~(0x1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandle->pGPIOx->OTYPER |= temp;
	temp=0;

	//5. Configure the alt functionality

	if(pGPIOHandle -> GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
	{
		//configure the alternate function registers
		uint32_t temp1 , temp2;

		temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
		temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;
		pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF<< (4*temp2));
		pGPIOHandle->pGPIOx->AFR[temp1] |= (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4*temp2));
	}


}
/*********************************************************************
 * @fn            - GPIO_PeriClockControl
 *
 * @brief         - This function enables or disables peripheral clock
 *                  for the given GPIO port
 *
 * @param[in]     - base address of the gpio peripheral
 * @param[in]     - ENABLE or DISABLE macros
 * @param[in]     -
 *
 * @return        - none
 *
 * @Note          - none
 *
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx)
{
			if(pGPIOx == CUSTOM_GPIOA)
			{
				GPIOA_REG_RESET();
			}
			else if(pGPIOx == CUSTOM_GPIOB)
			{
				GPIOB_REG_RESET();
			}
			else if(pGPIOx == CUSTOM_GPIOC)
					{
						GPIOC_REG_RESET();
					}
			else if(pGPIOx == CUSTOM_GPIOD)
					{
						GPIOD_REG_RESET();
					}
			else if(pGPIOx == CUSTOM_GPIOE)
					{
						GPIOE_REG_RESET();
					}
			else if(pGPIOx == CUSTOM_GPIOF)
					{
						GPIOF_REG_RESET();
					}
			else if(pGPIOx == CUSTOM_GPIOG)
					{
						GPIOG_REG_RESET();
					}
			else if(pGPIOx == CUSTOM_GPIOH)
					{
						GPIOH_REG_RESET();
					}
			else if(pGPIOx == CUSTOM_GPIOI)
					{
						GPIOI_REG_RESET();
					}
}
/*
 * Peripheral clock setup
 */
/*********************************************************************
 * @fn            - GPIO_PeriClockControl
 *
 * @brief         - This function enables or disables peripheral clock
 *                  for the given GPIO port
 *
 * @param[in]     - base address of the gpio peripheral
 * @param[in]     - ENABLE or DISABLE macros
 * @param[in]     -
 *
 * @return        - none
 *
 * @Note          - none
 *
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(pGPIOx == CUSTOM_GPIOA)
		{
			GPIOA_PCLK_EN();
		}
		else if(pGPIOx == CUSTOM_GPIOB)
		{
			GPIOB_PCLK_EN();
		}
		else if(pGPIOx == CUSTOM_GPIOC)
				{
					GPIOC_PCLK_EN();
				}
		else if(pGPIOx == CUSTOM_GPIOD)
				{
					GPIOD_PCLK_EN();
				}
		else if(pGPIOx == CUSTOM_GPIOE)
				{
					GPIOE_PCLK_EN();
				}
		else if(pGPIOx == CUSTOM_GPIOF)
				{
					GPIOF_PCLK_EN();
				}
		else if(pGPIOx == CUSTOM_GPIOG)
				{
					GPIOG_PCLK_EN();
				}
		else if(pGPIOx == CUSTOM_GPIOH)
				{
					GPIOH_PCLK_EN();
				}
		else if(pGPIOx == CUSTOM_GPIOI)
				{
					GPIOI_PCLK_EN();
				}

	}
	else
	{
		if(pGPIOx == CUSTOM_GPIOA)
				{
					GPIOA_PCLK_DI();
				}
				else if(pGPIOx == CUSTOM_GPIOB)
				{
					GPIOB_PCLK_DI();
				}
				else if(pGPIOx == CUSTOM_GPIOC)
						{
							GPIOC_PCLK_DI();
						}
				else if(pGPIOx == CUSTOM_GPIOD)
						{
							GPIOD_PCLK_DI();
						}
				else if(pGPIOx == CUSTOM_GPIOE)
						{
							GPIOE_PCLK_DI();
						}
				else if(pGPIOx == CUSTOM_GPIOF)
						{
							GPIOF_PCLK_DI();
						}
				else if(pGPIOx == CUSTOM_GPIOG)
						{
							GPIOG_PCLK_DI();
						}
				else if(pGPIOx == CUSTOM_GPIOH)
						{
							GPIOH_PCLK_DI();
						}
				else if(pGPIOx == CUSTOM_GPIOI)
						{
							GPIOI_PCLK_DI();
						}
	}

}
/*
 *  Data read and write
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)     // for pin use uint8_t , port use uint16_t
{
	uint8_t value;
	value = (uint8_t) ((pGPIOx->IDR >> PinNumber) & 0x00000001);
	return value;

}

uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx )
{
	uint16_t value;
	value = (uint16_t) (pGPIOx->IDR);
	return value;
}

void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value)
{
	if(Value == GPIO_PIN_SET)
	{
		pGPIOx->ODR |= (1 << PinNumber);
	}
	else
	{
		pGPIOx->ODR &= ~(1 << PinNumber);
	}

}
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value)
{
	pGPIOx->ODR = Value;
}
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
	pGPIOx -> ODR ^= (1<<PinNumber);
}
/*
 * 	IRQ
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber , uint8_t EnorDi)
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

void GPIO_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority)
{
	//find the IPR register
	uint8_t iprx = IRQNumber/4;
	uint8_t iprx_selection = IRQNumber %4;

	*(NVIC_PR_BASE_ADDR + iprx ) |= (IRQPriority << ((8*iprx_selection) + (8 - NO_PR_BIT_IMPLEMENTED)));
}
void GPIO_IRQHandling(uint8_t PinNumber)
{
	if (CUSTOM_EXTI->PR & (1U << PinNumber))
	{
	    CUSTOM_EXTI->PR |= (1U << PinNumber);
	}
}
