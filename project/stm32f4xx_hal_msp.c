/**
  ******************************************************************************
  * @file    stm32f4xx_hal_msp_template.c
  * @author  MCD Application Team
  * @brief   This file contains the HAL System and Peripheral (PPP) MSP initialization
  *          and de-initialization functions.
  *          It should be copied to the application folder and renamed into 'stm32f4xx_hal_msp.c'.           
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP HAL MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define  UART1_PORT                       GPIOA
#define  UART1_TX                         GPIO_PIN_9
#define  UART1_RX                         GPIO_PIN_10

#define  UART2_PORT                       GPIOA
#define  UART2_TX                         GPIO_PIN_2
#define  UART2_RX                         GPIO_PIN_3

#define  UART6_PORT                       GPIOC
#define  UART6_TX                         GPIO_PIN_6
#define  UART6_RX                         GPIO_PIN_7

#define  SPI1_PORT                        GPIOA
#define  SPI1_CLK                         GPIO_PIN_5
#define  SPI1_MISO                        GPIO_PIN_6
#define  SPI1_MOSI                        GPIO_PIN_7

#define  SPI2_PORT                        GPIOB
#define  SPI2_CLK                         GPIO_PIN_13
#define  SPI2_MISO                        GPIO_PIN_14
#define  SPI2_MOSI                        GPIO_PIN_15

#define  SPI3_PORT                        GPIOC
#define  SPI3_CLK                         GPIO_PIN_10
#define  SPI3_MISO                        GPIO_PIN_11
#define  SPI3_MOSI                        GPIO_PIN_12

#define  I2C1_PORT                        GPIOB
#define  I2C1_SCL                         GPIO_PIN_8
#define  I2C1_SDA                         GPIO_PIN_9

#define  I2C2_PORT                        GPIOB
#define  I2C2_SCL                         GPIO_PIN_10
#define  I2C2_SDA                         GPIO_PIN_11

#define  I2C3_SCL_PORT                    GPIOA
#define  I2C3_SCL                         GPIO_PIN_8
#define  I2C3_SDA_PORT                    GPIOC
#define  I2C3_SDA                         GPIO_PIN_9

#define  CAN1_PORT                        GPIOD
#define  CAN1_RX                          GPIO_PIN_0
#define  CAN1_TX                          GPIO_PIN_1

#define  CAN2_PORT                        GPIOB
#define  CAN2_RX                          GPIO_PIN_5
#define  CAN2_TX                          GPIO_PIN_6


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions HAL MSP Private Functions
  * @{
  */

/**
  * @brief  Initializes the Global MSP.
  * @note   This function is called from HAL_Init() function to perform system
  *         level initialization (GPIOs, clock, DMA, interrupt).
  * @retval None
  */
void HAL_MspInit(void)
{

}

/**
  * @brief  DeInitializes the Global MSP.
  * @note   This functiona is called from HAL_DeInit() function to perform system
  *         level de-initialization (GPIOs, clock, DMA, interrupt).
  * @retval None
  */
void HAL_MspDeInit(void)
{

}

/**
  * @brief  Initializes the PPP MSP.
  * @note   This functiona is called from HAL_PPP_Init() function to perform 
  *         peripheral(PPP) system level initialization (GPIOs, clock, DMA, interrupt)
  * @retval None
  */
/**
  * @brief  HAL_PPP_MspInit and HAL_PPP_MspDeInit.
  * @param  PPP_HandleTypeDef *hppp
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* UART1 clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();
        __HAL_RCC_USART1_FORCE_RESET();
        __HAL_RCC_USART1_RELEASE_RESET();
        
        /* UART1 gpio disable */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* UART1 DMA Stream disable */
        HAL_DMA_DeInit(huart->hdmarx); 
        HAL_DMA_DeInit(huart->hdmarx);

        /* UART1 interrupt disable */
        HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);
        HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn);
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else if (huart->Instance == USART2)
    {
        /* UART2 clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /* UART2 GPIO disable */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

        /* UART2 interrupt disable */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
    else if (huart->Instance == USART6)
    {
        /* UART6 clock disable */
        __HAL_RCC_USART6_CLK_DISABLE();

        /* UART6 GPIO disable */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);

        /* UART6 interrupt disable */
        HAL_NVIC_DisableIRQ(USART6_IRQn);
    }

    return;
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_UART_MspDeInit(huart);

    if (huart->Instance == USART1)
    {
        /* UART1 use DMA */
        static DMA_HandleTypeDef huart1_dma2_tx;
        static DMA_HandleTypeDef huart1_dma2_rx;

        /* UART1 clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();
        
        /* UART1 pin config */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* Configure the DMA handler for tx */
        huart1_dma2_tx.Instance = DMA2_Stream7;
        huart1_dma2_tx.Init.Channel = DMA_CHANNEL_4;
        huart1_dma2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        huart1_dma2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        huart1_dma2_tx.Init.MemInc = DMA_MINC_ENABLE;
        huart1_dma2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        huart1_dma2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        huart1_dma2_tx.Init.Mode = DMA_NORMAL;
        huart1_dma2_tx.Init.Priority = DMA_PRIORITY_LOW;
        huart1_dma2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        huart1_dma2_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        huart1_dma2_tx.Init.MemBurst = DMA_MBURST_INC4;
        huart1_dma2_tx.Init.PeriphBurst = DMA_PBURST_INC4;
        HAL_DMA_Init(&huart1_dma2_tx);   

        /* Configure the DMA handler for rx */
        huart1_dma2_rx.Instance = DMA2_Stream5;
        huart1_dma2_rx.Init.Channel = DMA_CHANNEL_4;
        huart1_dma2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        huart1_dma2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        huart1_dma2_rx.Init.MemInc = DMA_MINC_ENABLE;
        huart1_dma2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        huart1_dma2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        huart1_dma2_rx.Init.Mode = DMA_NORMAL;
        huart1_dma2_rx.Init.Priority = DMA_PRIORITY_HIGH;
        huart1_dma2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;         
        huart1_dma2_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        huart1_dma2_rx.Init.MemBurst = DMA_MBURST_INC4;
        huart1_dma2_rx.Init.PeriphBurst = DMA_PBURST_INC4; 
        HAL_DMA_Init(&huart1_dma2_rx);
        
        /* Associate the initialized DMA handle to the the UART handle */
        __HAL_LINKDMA(huart, hdmatx, huart1_dma2_tx);
        __HAL_LINKDMA(huart, hdmarx, huart1_dma2_rx);

        /* NVIC configuration for DMA tx interrupt */
        HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    
        /* NVIC configuration for DMA rx interrupt */
        HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);   
        HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

        /*Enable the uart Interrupt*/
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
        
        /* Clear event flag bit (xxIF) before open interrupt */
        __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);
        __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TXE);

        /* UART1 Set Priority and Enable */
        HAL_NVIC_SetPriority(USART1_IRQn, 9, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    else if (huart->Instance == USART2)
    {
        /* UART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* UART2 GPIO Configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /*Enable the uart Interrupt*/
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
        
        /* UART2 Set Priority and Enable */
        HAL_NVIC_SetPriority(USART2_IRQn, 9, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if (huart->Instance == USART6)
    {
        /* UART6 clock enable */
        __HAL_RCC_USART6_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        /* UART1 GPIO Configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /*Enable the uart Interrupt*/
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
        
        /* UART6 Set Priority and Enable */
        HAL_NVIC_SetPriority(USART6_IRQn, 9, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);
    }
    
    return;
}

#if 0
static void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        /* SPI1 clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /* SPI1 GPIO disable */
        HAL_GPIO_DeInit(SPI1_PORT, SPI1_CLK | SPI1_MISO | SPI1_MOSI);

        /* SPI1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(SPI1_IRQn);
    }
    else if (hspi->Instance == SPI2)
    {
        /* SPI2 clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /* SPI2 GPIO disable */
        HAL_GPIO_DeInit(SPI2_PORT, SPI2_CLK | SPI2_MISO | SPI2_MOSI);

        /* SPI2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(SPI2_IRQn);
    }
    else if (hspi->Instance == SPI3)
    {
        /* SPI3 clock disable */
        __HAL_RCC_SPI3_CLK_DISABLE();

        /* SPI3 GPIO disable */
        HAL_GPIO_DeInit(SPI3_PORT, SPI3_CLK | SPI3_MISO | SPI3_MOSI);

        /* SPI3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(SPI3_IRQn);
    }

    return;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_SPI_MspDeInit(hspi);

    if (hspi->Instance == SPI1)
    {
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* SPI1 GPIO Configuration */
        GPIO_InitStruct.Pin = SPI1_CLK | SPI1_MISO | SPI1_MOSI;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(SPI1_PORT, &GPIO_InitStruct);
    }
    else if (hspi->Instance == SPI2)
    {
        /* SPI2 clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* SPI2 GPIO Configuration */
        GPIO_InitStruct.Pin = SPI2_CLK | SPI2_MISO | SPI2_MOSI;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(SPI2_PORT, &GPIO_InitStruct);
    }
    else if (hspi->Instance == SPI3)
    {
        /* SPI3 clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        /* SPI3 GPIO Configuration */
        GPIO_InitStruct.Pin = SPI3_CLK | SPI3_MISO | SPI3_MOSI;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI3;
        HAL_GPIO_Init(SPI3_PORT, &GPIO_InitStruct);
    }

    return;
}

static void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        /* ADC1 clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

        /* ADC1 GPIO disable
           PB0  ------> ADC1_IN8
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
    }

    return;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_ADC_MspDeInit(hadc);

    if (hadc->Instance == ADC1)
    {
        /* ADC1 clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* ADC1 GPIO Configuration
           PB0  ------> ADC1_IN8
        */
        GPIO_InitStruct.Pin  = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }

    return;
}

static void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        /* TIM1 clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();

        /* TIM1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
    }
    else if (htim->Instance == TIM2)
    {
        /* TIM2 clock disable */
        __HAL_RCC_TIM2_CLK_DISABLE();

        /* TIM2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM2_IRQn);
    }
    else if (htim->Instance == TIM3)
    {
        /* TIM3 clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();

        /* TIM3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    }

    return;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    HAL_TIM_Base_MspDeInit(htim);

    if (htim->Instance == TIM1)
    {
        /* TIM1 clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();

        /* TIM1 Set Priority */
        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 1, 0);

        /*Enable the UPDATA Interrupt*/
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    }
    else if (htim->Instance == TIM2)
    {
        /* TIM2 clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();

        /* TIM2 Set Priority */
        HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);

        /*Enable the UPDATA Interrupt*/
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    }
    else if (htim->Instance == TIM3)
    {
        /* TIM3 clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();

        /* TIM3 Set Priority */
        HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);

        /*Enable the UPDATA Interrupt*/
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    }

    return;
}

static void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        /* I2C1 clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /* I2C1 GPIO disable */
        HAL_GPIO_DeInit(I2C1_PORT, I2C1_SCL | I2C1_SDA);

        /* I2C1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
    }
    else if (hi2c->Instance == I2C2)
    {
        /* I2C2 clock disable */
        __HAL_RCC_I2C2_CLK_DISABLE();

        /* I2C2 GPIO disable */
        HAL_GPIO_DeInit(I2C2_PORT, I2C2_SCL | I2C2_SDA);

        /* I2C2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
    }
    else if (hi2c->Instance == I2C3)
    {
        /* I2C3 clock disable */
        __HAL_RCC_I2C3_CLK_DISABLE();

        /* I2C3 GPIO disable */
        HAL_GPIO_DeInit(I2C3_SCL_PORT, I2C3_SCL);
        HAL_GPIO_DeInit(I2C3_SDA_PORT, I2C3_SDA);
        
        /* I2C3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
    }

    return;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_I2C_MspDeInit(hi2c);

    if (hi2c->Instance == I2C1)
    {
        /* I2C1 clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* I2C1 GPIO Configuration */
        GPIO_InitStruct.Pin = I2C1_SCL | I2C1_SDA;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(I2C1_PORT, &GPIO_InitStruct);
    }
    else if (hi2c->Instance == I2C2)
    {
        /* I2C2 clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* I2C2 GPIO Configuration */
        GPIO_InitStruct.Pin = I2C2_SCL | I2C2_SDA;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(I2C2_PORT, &GPIO_InitStruct);
    }
    else if (hi2c->Instance == I2C3)
    {
        /* I2C3 clock enable */
        __HAL_RCC_I2C3_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        
        /* I2C3 GPIO Configuration */
        GPIO_InitStruct.Pin = I2C3_SCL;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
        HAL_GPIO_Init(I2C3_SCL_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = I2C3_SDA;
        HAL_GPIO_Init(I2C3_SDA_PORT, &GPIO_InitStruct);
    }

    return;
}
#endif
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
