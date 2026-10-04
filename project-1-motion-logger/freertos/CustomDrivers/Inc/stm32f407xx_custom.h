/*
 * stm32f407xx.h
 *
 * Created on: Sep 23, 2026
 * Author: R.S. Hari Naveen
 */

#ifndef INC_STM32F407XX_CUSTOM_H_
#define INC_STM32F407XX_CUSTOM_H_

#include <stdint.h>
#include <stddef.h>
#define __vo volatile

/* ============================================================
 * NVIC ISERx register Address of ARM Cortex M4
 * ============================================================ */
#define NVIC_ISER0				((__vo uint32_t*)0xE000E100)
#define NVIC_ISER1				((__vo uint32_t*)0xE000E104)
#define NVIC_ISER2				((__vo uint32_t*)0xE000E108)
#define NVIC_ISER3				((__vo uint32_t*)0xE000E10C)

/* ============================================================
 * NVIC ICERx register Address of ARM Cortex M4
 * ============================================================ */
#define NVIC_ICER0				((__vo uint32_t*)0xE000E180)
#define NVIC_ICER1				((__vo uint32_t*)0xE000E184)
#define NVIC_ICER2				((__vo uint32_t*)0xE000E188)
#define NVIC_ICER3				((__vo uint32_t*)0xE000E18C)

/* ============================================================
 * NVIC Priority base address
 * ============================================================ */

#define NVIC_PR_BASE_ADDR 		((__vo uint32_t*)0xE000E400)
#define NO_PR_BIT_IMPLEMENTED	4

/* ============================================================
 * Memory base addresses
 * ============================================================ */

#define FLASH_BASEADDR          0x08000000U
#define SRAM1_BASEADDR          0x20000000U  /* 112 KB */
#define SRAM2_BASEADDR          0x2001C000U  /* 16 KB */
#define SRAM                    SRAM1_BASEADDR
#define ROM                     0x1FFF0000U /* System memory */

/* ============================================================
 * AHB and APB bus base addresses
 * ============================================================ */

#define PERIPH_BASEADDR         0x40000000U
#define APB1PERIPH_BASEADDR      PERIPH_BASEADDR
#define APB2PERIPH_BASEADDR      0x40010000U
#define AHB1PERIPH_BASEADDR      0x40020000U
#define AHB2PERIPH_BASEADDR      0x50000000U

/* ============================================================
 * AHB1 peripheral base addresses
 * ============================================================ */

#define GPIOA_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0000U)
#define GPIOB_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0400U)
#define GPIOC_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0800U)
#define GPIOD_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0C00U)
#define GPIOE_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1000U)
#define GPIOF_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1400U)
#define GPIOG_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1800U)
#define GPIOH_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1C00U)
#define GPIOI_BASEADDR          (AHB1PERIPH_BASEADDR + 0x2000U)

#define RCC_BASEADDR            (AHB1PERIPH_BASEADDR + 0x3800U)

/* ============================================================
 * APB1 peripheral base addresses
 * ============================================================ */

#define SPI2_BASEADDR           (APB1PERIPH_BASEADDR + 0x3800U)
#define SPI3_BASEADDR           (APB1PERIPH_BASEADDR + 0x3C00U)

#define USART2_BASEADDR         (APB1PERIPH_BASEADDR + 0x4400U)
#define USART3_BASEADDR         (APB1PERIPH_BASEADDR + 0x4800U)
#define UART4_BASEADDR          (APB1PERIPH_BASEADDR + 0x4C00U)
#define UART5_BASEADDR          (APB1PERIPH_BASEADDR + 0x5000U)

#define I2C1_BASEADDR           (APB1PERIPH_BASEADDR + 0x5400U)
#define I2C2_BASEADDR           (APB1PERIPH_BASEADDR + 0x5800U)
#define I2C3_BASEADDR           (APB1PERIPH_BASEADDR + 0x5C00U)

/* ============================================================
 * APB2 peripheral base addresses
 * ============================================================ */

#define USART1_BASEADDR         (APB2PERIPH_BASEADDR + 0x1000U)
#define USART6_BASEADDR         (APB2PERIPH_BASEADDR + 0x1400U)

#define SPI1_BASEADDR           (APB2PERIPH_BASEADDR + 0x3000U)
#define SPI4_BASEADDR			(APB2PERIPH_BASEADDR + 0x3400U)

#define SYSCFG_BASEADDR         (APB2PERIPH_BASEADDR + 0x3800U)
#define EXTI_BASEADDR           (APB2PERIPH_BASEADDR + 0x3C00U)

/* ============================================================
 * GPIO register definition
 * ============================================================ */

typedef struct
{
    __vo uint32_t MODER;         /* Offset: 0x00 */
    __vo uint32_t OTYPER;        /* Offset: 0x04 */
    __vo uint32_t OSPEEDR;       /* Offset: 0x08 */
    __vo uint32_t PUPDR;         /* Offset: 0x0C */
    __vo uint32_t IDR;           /* Offset: 0x10 */
    __vo uint32_t ODR;           /* Offset: 0x14 */
    __vo uint32_t BSRR;          /* Offset: 0x18 */
    __vo uint32_t LCKR;          /* Offset: 0x1C */
    __vo uint32_t AFR[2];        /* AFR[0]: 0x20, AFR[1]: 0x24 */

} GPIO_RegDef_t;

/* ============================================================
 * CUSTOM_RCC register definition — STM32F407
 * ============================================================ */

typedef struct
{
    __vo uint32_t CR;            /* Offset: 0x00 */
    __vo uint32_t PLLCFGR;       /* Offset: 0x04 */
    __vo uint32_t CFGR;          /* Offset: 0x08 */
    __vo uint32_t CIR;           /* Offset: 0x0C */

    __vo uint32_t AHB1RSTR;      /* Offset: 0x10 */
    __vo uint32_t AHB2RSTR;      /* Offset: 0x14 */
    __vo uint32_t AHB3RSTR;      /* Offset: 0x18 */
    uint32_t RESERVED0;         /* Offset: 0x1C */

    __vo uint32_t APB1RSTR;      /* Offset: 0x20 */
    __vo uint32_t APB2RSTR;      /* Offset: 0x24 */
    uint32_t RESERVED1[2];      /* Offsets: 0x28, 0x2C */

    __vo uint32_t AHB1ENR;       /* Offset: 0x30 */
    __vo uint32_t AHB2ENR;       /* Offset: 0x34 */
    __vo uint32_t AHB3ENR;       /* Offset: 0x38 */
    uint32_t RESERVED2;         /* Offset: 0x3C */

    __vo uint32_t APB1ENR;       /* Offset: 0x40 */
    __vo uint32_t APB2ENR;       /* Offset: 0x44 */
    uint32_t RESERVED3[2];      /* Offsets: 0x48, 0x4C */

    __vo uint32_t AHB1LPENR;     /* Offset: 0x50 */
    __vo uint32_t AHB2LPENR;     /* Offset: 0x54 */
    __vo uint32_t AHB3LPENR;     /* Offset: 0x58 */
    uint32_t RESERVED4;         /* Offset: 0x5C */

    __vo uint32_t APB1LPENR;     /* Offset: 0x60 */
    __vo uint32_t APB2LPENR;     /* Offset: 0x64 */
    uint32_t RESERVED5[2];      /* Offsets: 0x68, 0x6C */

    __vo uint32_t BDCR;          /* Offset: 0x70 */
    __vo uint32_t CSR;           /* Offset: 0x74 */
    uint32_t RESERVED6[2];      /* Offsets: 0x78, 0x7C */

    __vo uint32_t SSCGR;         /* Offset: 0x80 */
    __vo uint32_t PLLI2SCFGR;    /* Offset: 0x84 */

} RCC_RegDef_t;


/* ============================================================
 *  EXTI register definition — STM32F407
 * ============================================================ */

typedef struct
{
    __vo uint32_t IMR;         /* Offset: 0x00 */
    __vo uint32_t EMR;        /* Offset: 0x04 */
    __vo uint32_t RTSR;       /* Offset: 0x08 */
    __vo uint32_t FTSR;         /* Offset: 0x0C */
    __vo uint32_t SWIER;           /* Offset: 0x10 */
    __vo uint32_t PR;           /* Offset: 0x14 */

} EXTI_RegDef_t;


/* ============================================================
 * SYSCFG peripheral  register definition
 * ============================================================ */

typedef struct
{
    __vo uint32_t MEMRMP;         /* Offset: 0x00 */
    __vo uint32_t PMC;        /* Offset: 0x04 */
    __vo uint32_t EXTICR[4];           /* Offset:0x08 to 0x14 */
    uint32_t RESERVED[2];          /* Offset: 0x18 to 0x1C */
    __vo uint32_t CMPCR;          /* Offset: 0x20 */

} SYSCFG_RegDef_t;

/* ============================================================
 * SPI peripheral register definition — STM32F407
 * ============================================================ */

typedef struct
{
	__vo uint32_t CR1;
	__vo uint32_t CR2;
	__vo uint32_t SR;
	__vo uint32_t DR;
	__vo uint32_t CRCPR;
	__vo uint32_t RXCRCR;
	__vo uint32_t TXCRCR;
	__vo uint32_t I2SCFGR;
	__vo uint32_t I2SPR;

} SPI_RegDef_t;

/* ============================================================
 * USART register definition
 * ============================================================ */
typedef struct
{
	__vo uint32_t SR;
	__vo uint32_t DR;
	__vo uint32_t BRR;
	__vo uint32_t CR1;
	__vo uint32_t CR2;
	__vo uint32_t CR3;
	__vo uint32_t GTPR;

}USART_RegDef_t;


/* ============================================================
 * Peripheral pointer definitions
 * ============================================================ */

#define CUSTOM_GPIOA                   ((GPIO_RegDef_t *)GPIOA_BASEADDR)
#define CUSTOM_GPIOB                   ((GPIO_RegDef_t *)GPIOB_BASEADDR)
#define CUSTOM_GPIOC                   ((GPIO_RegDef_t *)GPIOC_BASEADDR)
#define CUSTOM_GPIOD                   ((GPIO_RegDef_t *)GPIOD_BASEADDR)
#define CUSTOM_GPIOE                   ((GPIO_RegDef_t *)GPIOE_BASEADDR)
#define CUSTOM_GPIOF                   ((GPIO_RegDef_t *)GPIOF_BASEADDR)
#define CUSTOM_GPIOG                   ((GPIO_RegDef_t *)GPIOG_BASEADDR)
#define CUSTOM_GPIOH                   ((GPIO_RegDef_t *)GPIOH_BASEADDR)
#define CUSTOM_GPIOI                   ((GPIO_RegDef_t *)GPIOI_BASEADDR)

#define CUSTOM_RCC                     ((RCC_RegDef_t *)RCC_BASEADDR)
#define CUSTOM_EXTI 					((EXTI_RegDef_t *)EXTI_BASEADDR)
#define CUSTOM_SYSCFG					((SYSCFG_RegDef_t *)SYSCFG_BASEADDR)

#define CUSTOM_SPI1					((SPI_RegDef_t *)SPI1_BASEADDR)
#define CUSTOM_SPI2					((SPI_RegDef_t *)SPI2_BASEADDR)
#define CUSTOM_SPI3					((SPI_RegDef_t *)SPI3_BASEADDR)
#define CUSTOM_SPI4					((SPI_RegDef_t *)SPI4_BASEADDR)

#define CUSTOM_USART1					((USART_RegDef_t *)USART1_BASEADDR)
#define CUSTOM_USART2					((USART_RegDef_t *)USART2_BASEADDR)
#define CUSTOM_USART3					((USART_RegDef_t *)USART3_BASEADDR)
#define CUSTOM_UART4					((USART_RegDef_t *)UART4_BASEADDR)
#define CUSTOM_UART5					((USART_RegDef_t *)UART5_BASEADDR)
#define CUSTOM_USART6					((USART_RegDef_t *)USART6_BASEADDR)


/*
 * Clock enable macro for the GPIOx peripherals
 */

#define GPIOA_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<0))
#define GPIOB_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<1))
#define GPIOC_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<2))
#define GPIOD_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<3))
#define GPIOE_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<4))
#define GPIOF_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<5))
#define GPIOG_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<6))
#define GPIOH_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<7))
#define GPIOI_PCLK_EN() 	(CUSTOM_RCC -> AHB1ENR |= (1<<8))

/*
 *  Clock enable macro for the I2Cx peripherals
 */

#define I2C1_PCLK_EN()		(CUSTOM_RCC-> APB1ENR |= (1<<21))
#define I2C2_PCLK_EN()		(CUSTOM_RCC-> APB1ENR |= (1<<22))
#define I2C3_PCLK_EN()		(CUSTOM_RCC-> APB1ENR |= (1<<23))

/*
 *  Clock enable macro for the SPIx peripherals
 */

#define SPI1_PCLK_EN()		(CUSTOM_RCC-> APB2ENR |= (1<<12))
#define SPI2_PCLK_EN()		(CUSTOM_RCC-> APB1ENR |= (1<<14))
#define SPI3_PCLK_EN()		(CUSTOM_RCC-> APB1ENR |= (1<<15))
#define SPI4_PCLK_EN()		(CUSTOM_RCC-> APB2ENR |= (1<<13))

/*
 * Clock enable macro for the USARTx peripherals
 */

#define USART1_PCLK_EN() 	(CUSTOM_RCC-> APB2ENR |= (1<<4))
#define USART2_PCLK_EN() 	(CUSTOM_RCC-> APB1ENR |= (1<<17))
#define USART3_PCLK_EN() 	(CUSTOM_RCC-> APB1ENR |= (1<<18))
#define UART4_PCLK_EN() 	(CUSTOM_RCC-> APB1ENR |= (1<<19))
#define UART5_PCLK_EN() 	(CUSTOM_RCC-> APB1ENR |= (1<<20))
#define USART6_PCLK_EN() 	(CUSTOM_RCC-> APB2ENR |= (1<<5))

/*
 * Clock enable macro for the SYSCFG peripherals
 */

#define SYSCFG_PCLK_EN()	(CUSTOM_RCC-> APB2ENR |= (1<<14))


/*
 * Clock DISABLE macro for the GPIOx peripherals
 */

#define GPIOA_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<0))
#define GPIOB_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<1))
#define GPIOC_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<2))
#define GPIOD_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<3))
#define GPIOE_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<4))
#define GPIOF_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<5))
#define GPIOG_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<6))
#define GPIOH_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<7))
#define GPIOI_PCLK_DI() 	(CUSTOM_RCC -> AHB1ENR &= ~(1<<8))

/*
 *  Clock disable macro for the I2Cx peripherals
 */

#define I2C1_PCLK_DI()		(CUSTOM_RCC-> APB1ENR &= ~(1<<21))
#define I2C2_PCLK_DI()		(CUSTOM_RCC-> APB1ENR &= ~(1<<22))
#define I2C3_PCLK_DI()		(CUSTOM_RCC-> APB1ENR &= ~(1<<23))

/*
 *  Clock disable macro for the SPIx peripherals
 */

#define SPI1_PCLK_DI()		(CUSTOM_RCC-> APB2ENR &= ~(1<<12))
#define SPI2_PCLK_DI()		(CUSTOM_RCC-> APB1ENR &= ~(1<<14))
#define SPI3_PCLK_DI()		(CUSTOM_RCC-> APB1ENR &= ~(1<<15))
#define SPI4_PCLK_DI()		(CUSTOM_RCC-> APB2ENR &= ~(1<<13))

/*
 * Clock disable macro for the USARTx peripherals
 */

#define USART1_PCLK_DI() 	(CUSTOM_RCC-> APB2ENR &= ~(1<<4))
#define USART2_PCLK_DI() 	(CUSTOM_RCC-> APB1ENR &= ~(1<<17))
#define USART3_PCLK_DI() 	(CUSTOM_RCC-> APB1ENR &= ~(1<<18))
#define UART4_PCLK_DI() 	(CUSTOM_RCC-> APB1ENR &= ~(1<<19))
#define UART5_PCLK_DI() 	(CUSTOM_RCC-> APB1ENR &= ~(1<<20))
#define USART6_PCLK_DI() 	(CUSTOM_RCC-> APB2ENR &= ~(1<<5))

/*
 * Clock disable macro for the SYSCFG peripherals
 */

#define SYSCFG_PCLK_DI()	(CUSTOM_RCC-> APB2ENR &= ~(1<<14))


/*
 *  Macros to reset GPIOx peripherals
 */

#define GPIOA_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<0) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<0); } while(0)
#define GPIOB_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<1) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<1); } while(0)
#define GPIOC_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<2) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<2); } while(0)
#define GPIOD_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<3) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<3); } while(0)
#define GPIOE_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<4) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<4); } while(0)
#define GPIOF_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<5) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<5); } while(0)
#define GPIOG_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<6) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<6); } while(0)
#define GPIOH_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<7) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<7); } while(0)
#define GPIOI_REG_RESET()	do{  CUSTOM_RCC->AHB1RSTR |= (1<<8) ; CUSTOM_RCC->AHB1RSTR &= ~(1<<8); } while(0)


/*
 *  Macros to reset SPIx peripherals
 */

#define SPI1_REG_RESET()	do{  CUSTOM_RCC->APB2RSTR |= (1<<12) ; CUSTOM_RCC->APB2RSTR &= ~(1<<12); } while(0)
#define SPI2_REG_RESET()	do{  CUSTOM_RCC->APB1RSTR |= (1<<14) ; CUSTOM_RCC->APB1RSTR &= ~(1<<14); } while(0)
#define SPI3_REG_RESET()	do{  CUSTOM_RCC->APB1RSTR |= (1<<15) ; CUSTOM_RCC->APB1RSTR &= ~(1<<15); } while(0)
#define SPI4_REG_RESET()	do{  CUSTOM_RCC->APB2RSTR |= (1<<13) ; CUSTOM_RCC->APB2RSTR &= ~(1<<13); } while(0)

/*
 *  Macros to reset USARTx peripherals
 */
#define USART1_REG_RESET()	do{  CUSTOM_RCC->APB2RSTR |= (1<<4) ; CUSTOM_RCC->APB2RSTR &= ~(1<<4); } while(0)
#define USART2_REG_RESET()	do{  CUSTOM_RCC->APB1RSTR |= (1<<17) ; CUSTOM_RCC->APB1RSTR &= ~(1<<17); } while(0)
#define USART3_REG_RESET()	do{  CUSTOM_RCC->APB1RSTR |= (1<<18) ; CUSTOM_RCC->APB1RSTR &= ~(1<<18); } while(0)
#define UART4_REG_RESET()	do{  CUSTOM_RCC->APB1RSTR |= (1<<19) ; CUSTOM_RCC->APB1RSTR &= ~(1<<19); } while(0)
#define UART5_REG_RESET()	do{  CUSTOM_RCC->APB1RSTR |= (1<<20) ; CUSTOM_RCC->APB1RSTR &= ~(1<<20); } while(0)
#define USART6_REG_RESET()	do{  CUSTOM_RCC->APB2RSTR |= (1<<5) ; CUSTOM_RCC->APB2RSTR &= ~(1<<5); } while(0)


/*
 * returns the port code for a given GPIOx Base address
 */

#define GPIO_BASEADDR_TO_CODE(x)	(x==CUSTOM_GPIOA)?0:\
									(x==CUSTOM_GPIOB)?1:\
									(x==CUSTOM_GPIOC)?2:\
									(x==CUSTOM_GPIOD)?3:\
									(x==CUSTOM_GPIOE)?4:\
									(x==CUSTOM_GPIOF)?5:\
									(x==CUSTOM_GPIOG)?6:\
									(x==CUSTOM_GPIOH)?7:\
									(x==CUSTOM_GPIOI)?8:0

/*
 * IRQ(Interrupt Request) Number of STM32F407x MCU
 */

#define IRQ_NO_EXTI0       6
#define IRQ_NO_EXTI1       7
#define IRQ_NO_EXTI2       8
#define IRQ_NO_EXTI3       9
#define IRQ_NO_EXTI4       10
#define IRQ_NO_EXTI9_5     23h
#define IRQ_NO_EXTI15_10   40

#define IRQ_NO_SPI1			35
#define IRQ_NO_SPI2			36
#define IRQ_NO_SPI3			51

/* Bit position definitions: SPI_CR1 */
#define CUSTOM_SPI_CR1_CPHA       0
#define CUSTOM_SPI_CR1_CPOL       1
#define CUSTOM_SPI_CR1_MSTR       2
#define CUSTOM_SPI_CR1_BR         3
#define CUSTOM_SPI_CR1_SPE        6
#define CUSTOM_SPI_CR1_LSBFIRST   7
#define CUSTOM_SPI_CR1_SSI        8
#define CUSTOM_SPI_CR1_SSM        9
#define CUSTOM_SPI_CR1_RXONLY     10
#define CUSTOM_SPI_CR1_DFF        11
#define CUSTOM_SPI_CR1_CRCNEXT    12
#define CUSTOM_SPI_CR1_CRCEN      13
#define CUSTOM_SPI_CR1_BIDIOE     14
#define CUSTOM_SPI_CR1_BIDIMODE   15

/* Bit position definitions: SPI_CR2 */
#define CUSTOM_SPI_CR2_RXDMAEN    0
#define CUSTOM_SPI_CR2_TXDMAEN    1
#define CUSTOM_SPI_CR2_SSOE       2
#define CUSTOM_SPI_CR2_FRF        4
#define CUSTOM_SPI_CR2_ERRIE      5
#define CUSTOM_SPI_CR2_RXNEIE     6
#define CUSTOM_SPI_CR2_TXEIE      7

/* Bit position definitions: SPI_SR */
#define CUSTOM_SPI_SR_RXNE        0
#define CUSTOM_SPI_SR_TXE         1
#define CUSTOM_SPI_SR_CHSIDE      2
#define CUSTOM_SPI_SR_UDR         3
#define CUSTOM_SPI_SR_CRCERR      4
#define CUSTOM_SPI_SR_MODF        5
#define CUSTOM_SPI_SR_OVR         6
#define CUSTOM_SPI_SR_BSY         7
#define CUSTOM_SPI_SR_FRE         8
//some generic macros

#define ENABLE 			1
#define DISABLE 		0
#define SET 			ENABLE
#define RESET			DISABLE
#define GPIO_PIN_SET	SET
#define GPIO_PIN_RESET  RESET
#define FLAG_SET		SET
#define FLAG_RESET		RESET

/******************************************************************************************
 *Bit position definitions of USART peripheral
 ******************************************************************************************/

/*
 * Bit position definitions USART_CR1
 */
#define CUSTOM_USART_CR1_SBK					0
#define CUSTOM_USART_CR1_RWU 					1
#define CUSTOM_USART_CR1_RE  					2
#define CUSTOM_USART_CR1_TE 					3
#define CUSTOM_USART_CR1_IDLEIE 				4
#define CUSTOM_USART_CR1_RXNEIE  				5
#define CUSTOM_USART_CR1_TCIE					6
#define CUSTOM_USART_CR1_TXEIE					7
#define CUSTOM_USART_CR1_PEIE 					8
#define CUSTOM_USART_CR1_PS 					9
#define CUSTOM_USART_CR1_PCE 					10
#define CUSTOM_USART_CR1_WAKE  				11
#define CUSTOM_USART_CR1_M 					12
#define CUSTOM_USART_CR1_UE 					13
#define CUSTOM_USART_CR1_OVER8  				15



/*
 * Bit position definitions USART_CR2
 */
#define CUSTOM_USART_CR2_ADD   				0
#define CUSTOM_USART_CR2_LBDL   				5
#define CUSTOM_USART_CR2_LBDIE  				6
#define CUSTOM_USART_CR2_LBCL   				8
#define CUSTOM_USART_CR2_CPHA   				9
#define CUSTOM_USART_CR2_CPOL   				10
#define CUSTOM_USART_CR2_STOP   				12
#define CUSTOM_USART_CR2_LINEN   				14


/*
 * Bit position definitions USART_CR3
 */
#define CUSTOM_USART_CR3_EIE   				0
#define CUSTOM_USART_CR3_IREN   				1
#define CUSTOM_USART_CR3_IRLP  				2
#define CUSTOM_USART_CR3_HDSEL   				3
#define CUSTOM_USART_CR3_NACK   				4
#define CUSTOM_USART_CR3_SCEN   				5
#define CUSTOM_USART_CR3_DMAR  				6
#define CUSTOM_USART_CR3_DMAT   				7
#define CUSTOM_USART_CR3_RTSE   				8
#define CUSTOM_USART_CR3_CTSE   				9
#define CUSTOM_USART_CR3_CTSIE   				10
#define CUSTOM_USART_CR3_ONEBIT   				11

/*
 * Bit position definitions USART_SR
 */

#define CUSTOM_USART_SR_PE        				0
#define CUSTOM_USART_SR_FE        				1
#define CUSTOM_USART_SR_NE        				2
#define CUSTOM_USART_SR_ORE       				3
#define CUSTOM_USART_SR_IDLE       			4
#define CUSTOM_USART_SR_RXNE        			5
#define CUSTOM_USART_SR_TC        				6
#define CUSTOM_USART_SR_TXE        			7
#define CUSTOM_USART_SR_LBD        			8
#define CUSTOM_USART_SR_CTS        			9


#include "stm32f407x_gpio_driver.h"
#include "stm32f407xx_spi_driver.h"
#include "stm32f407x_usart_driver.h"
#include "stm32f407xx_rcc_driver.h"
#endif /* INC_STM32F407XX_CUSTOMH_ */
