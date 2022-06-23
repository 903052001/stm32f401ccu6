/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.c
  * @author  MCD Application Team
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "usbd_desc.h"
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Macros
  * @{
  */

/**
  * @}
  */
#define APP_RX_DATA_SIZE  1000
#define APP_TX_DATA_SIZE  1000
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
extern USBD_HandleTypeDef USBD_Device;
/* USER CODE BEGIN PRIVATE_VARIABLES */
USBD_HandleTypeDef USBD_Device;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
#define DEVICE_FS 0
#define DEVICE_HS 1
/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
    /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */

    /* USER CODE END USB_DEVICE_Init_PreTreatment */

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&USBD_Device, &CDC_Desc, 0) != USBD_OK)
    {
        assert_failed(__FILE__, __LINE__);
    }
    if (USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS) != USBD_OK)
    {
        assert_failed(__FILE__, __LINE__);
    }
    if (USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_if_fops) != USBD_OK)
    {
        assert_failed(__FILE__, __LINE__);
    }
    if (USBD_Start(&USBD_Device) != USBD_OK)
    {
        assert_failed(__FILE__, __LINE__);
    }
    /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

    /* USER CODE END USB_DEVICE_Init_PostTreatment */
}
/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */

static int8_t USBD_CDC_Itf_Init(void);
static int8_t USBD_CDC_Itf_DeInit(void);
static int8_t USBD_CDC_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t USBD_CDC_Itf_Receive(uint8_t *pbuf, uint32_t *Len);
static int8_t USBD_CDC_Itf_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

USBD_CDC_ItfTypeDef USBD_CDC_if_fops =
{
    USBD_CDC_Itf_Init,
    USBD_CDC_Itf_DeInit,
    USBD_CDC_Itf_Control,
    USBD_CDC_Itf_Receive,
    USBD_CDC_Itf_TransmitCplt
};

USBD_CDC_LineCodingTypeDef linecoding =
{
    9600,   /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  USBD_CDC_Itf_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Itf_Init(void)
{
    /*
       Add your initialization code here
    */
    USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBufferFS, 0);
    USBD_CDC_SetRxBuffer(&USBD_Device, UserRxBufferFS);

    return USBD_OK;
}

/**
  * @brief  USBD_CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Itf_DeInit(void)
{
    /*
       Add your deinitialization code here
    */
    USBD_CDC_SetTxBuffer(&USBD_Device, NULL, 0);
    USBD_CDC_SetRxBuffer(&USBD_Device, NULL);

    return USBD_OK;
}


/**
  * @brief  USBD_CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
    UNUSED(length);
    switch (cmd)
    {
    case CDC_SEND_ENCAPSULATED_COMMAND:
        /* Add your code here */
        break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
        /* Add your code here */
        break;

    case CDC_SET_COMM_FEATURE:
        /* Add your code here */
        break;

    case CDC_GET_COMM_FEATURE:
        /* Add your code here */
        break;

    case CDC_CLEAR_COMM_FEATURE:
        /* Add your code here */
        break;

    /*******************************************************************************/
    /* Line Coding Structure                                                       */
    /*-----------------------------------------------------------------------------*/
    /* Offset | Field       | Size | Value  | Description                          */
    /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
    /* 4      | bCharFormat |   1  | Number | Stop bits                            */
    /*                                        0 - 1 Stop bit                       */
    /*                                        1 - 1.5 Stop bits                    */
    /*                                        2 - 2 Stop bits                      */
    /* 5      | bParityType |  1   | Number | Parity                               */
    /*                                        0 - None                             */
    /*                                        1 - Odd                              */
    /*                                        2 - Even                             */
    /*                                        3 - Mark                             */
    /*                                        4 - Space                            */
    /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
    /*******************************************************************************/
    case CDC_SET_LINE_CODING:
        linecoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
        linecoding.format     = pbuf[4];
        linecoding.paritytype = pbuf[5];
        linecoding.datatype   = pbuf[6];

        /* Add your code here */
        break;

    case CDC_GET_LINE_CODING:
        pbuf[0] = (uint8_t)(linecoding.bitrate);
        pbuf[1] = (uint8_t)(linecoding.bitrate >> 8);
        pbuf[2] = (uint8_t)(linecoding.bitrate >> 16);
        pbuf[3] = (uint8_t)(linecoding.bitrate >> 24);
        pbuf[4] = linecoding.format;
        pbuf[5] = linecoding.paritytype;
        pbuf[6] = linecoding.datatype;

        /* Add your code here */
        break;

    case CDC_SET_CONTROL_LINE_STATE:
        /* Add your code here */
        break;

    case CDC_SEND_BREAK:
        /* Add your code here */
        break;

    default:
        break;
    }

    return USBD_OK;
}

/**
  * @brief  USBD_CDC_Itf_Receive
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Itf_Receive(uint8_t *Buf, uint32_t *Len)
{
    UNUSED(Buf);
    UNUSED(Len);

    USBD_CDC_SetRxBuffer(&USBD_Device, &Buf[0]);
    USBD_CDC_ReceivePacket(&USBD_Device);

    return USBD_OK;
}

/**
  * @brief  USBD_CDC_Itf_TransmitCplt
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Itf_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    UNUSED(Buf);
    UNUSED(Len);
    UNUSED(epnum);

    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 7 */
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)USBD_Device.pClassData;
    if (hcdc->TxState != 0)
    {
        return USBD_BUSY;
    }
//    USBD_CDC_SetTxBuffer(&USBD_Device, Buf, *Len);
//    result = USBD_CDC_TransmitPacket(&USBD_Device);
usb_printf("HHHHHH\r\n", sizeof("HHHHHH\r\n"));
    return (USBD_OK);
}

void usb_printf(uint8_t *Buf, uint32_t Len)
{
    uint32_t len = Len;
//USBD_CDC_Itf_TransmitCplt(Buf, &len, 0);

    USBD_CDC_SetTxBuffer(&USBD_Device, Buf, Len);
    USBD_CDC_TransmitPacket(&USBD_Device);

}
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

