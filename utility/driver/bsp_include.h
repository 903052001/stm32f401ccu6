/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_include.h                                                    */
/* 内容摘要: bsp头文件包含关系                                                */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef BSP_INCLUDE_H
#define BSP_INCLUDE_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/

/******************************************************************************/
/*                              其他条件编译选项                              */
/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/*                                  常量定义                                  */
/******************************************************************************/


/******************************************************************************/
/*                                 全局宏定义                                 */
/******************************************************************************/
#define BSP_USE_GPIO
#define BSP_USE_UART
#define BSP_USE_TIME
#define BSP_USE_I2C
#define BSP_USE_SPI
#define BSP_USE_FLASH
//#define BSP_USE_ADC
//#define BSP_USE_DAC
//#define BSP_USE_DCMI
//#define BSP_USE_IWDG
//#define BSP_USE_CAN
//#define BSP_USE_USB
//#define BSP_USE_RTC
//#define BSP_USE_SDIO
//#define BSP_USE_HASH
//#define BSP_USE_RNG
//#define BSP_USE_CRYP
//#define BSP_USE_MAC
//#define BSP_USE_FSMC
//#define BSP_USE_DBG

#define DEV_USE_CONSOLE
#define DEV_USE_W25Qxx
//#define DEV_USE_AT24Cxx
//#define DEV_USE_GY30
//#define DEV_USE_MPUxx

#ifdef BSP_USE_GPIO
#include "bsp_gpio.h"
#endif
#ifdef BSP_USE_UART
#include "bsp_uart.h"
#endif
#ifdef BSP_USE_ADC
#include "bsp_adc.h"
#endif
#ifdef BSP_USE_DAC
#include "bsp_dac.h"
#endif
#ifdef BSP_USE_DCMI
#include "bsp_dcmi.h"
#endif
#ifdef BSP_USE_TIME
#include "bsp_time.h"
#endif
#ifdef BSP_USE_IWDG
#include "bsp_iwdg.h"
#endif
#ifdef BSP_USE_CAN
#include "bsp_can.h"
#endif
#ifdef BSP_USE_I2C
#include "bsp_i2c.h"
#endif
#ifdef BSP_USE_SPI
#include "bsp_spi.h"
#endif
#ifdef BSP_USE_USB
#include "bsp_usb.h"
#endif
#ifdef BSP_USE_FLASH
#include "bsp_flash.h"
#endif
#ifdef BSP_USE_RTC
#include "bsp_rtc.h"
#endif
#ifdef BSP_USE_SDIO
#include "bsp_sdio.h"
#endif
#ifdef BSP_USE_HASH
#include "bsp_hash.h"
#endif
#ifdef BSP_USE_RNG
#include "bsp_rng.h"
#endif
#ifdef BSP_USE_CRYP
#include "bsp_cryp.h"
#endif
#ifdef BSP_USE_MAC
#include "bsp_mac.h"
#endif
#ifdef BSP_USE_FSMC
#include "bsp_fsmc.h"
#endif
#ifdef BSP_USE_DBG
#include "bsp_dbg.h"
#endif

#ifdef DEV_USE_CONSOLE
#include "drv_console.h"
#endif
#ifdef DEV_USE_AT24Cxx
#include "drv_at24cxx.h"
#endif
#ifdef DEV_USE_W25Qxx
#include "drv_w25qxx.h"
#endif
#ifdef DEV_USE_GY30
#include "drv_gy30.h"
#endif
#ifdef DEV_USE_MPUxx
#include "drv_mpuxx.h"
#endif

#include "Fn_common.h"

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int bsp_peripheral_init(void)
{
#ifdef BSP_USE_GPIO
    bsp_gpio_init();
#endif
#ifdef BSP_USE_UART
    bsp_uart_init();
#endif
#ifdef BSP_USE_ADC
    bsp_adc_init();
#endif
#ifdef BSP_USE_DAC
    bsp_dac_init();
#endif
#ifdef BSP_USE_DCMI
    bsp_dcmi_init();
#endif
#ifdef BSP_USE_TIME
    bsp_time_init();
#endif
#ifdef BSP_USE_IWDG
    bsp_iwdg_init();
#endif
#ifdef BSP_USE_CAN
    bsp_can_init();
#endif
#ifdef BSP_USE_I2C
    bsp_i2c_init();
#endif
#ifdef BSP_USE_SPI
    bsp_spi_init();
#endif
#ifdef BSP_USE_USB
    bsp_usb_init();
#endif
#ifdef BSP_USE_RTC
    bsp_rtc_init();
#endif
#ifdef BSP_USE_SDIO
    bsp_sdio_init();
#endif
#ifdef BSP_USE_HASH
    bsp_hash_init();
#endif
#ifdef BSP_USE_RNG
    bsp_rng_init();
#endif
#ifdef BSP_USE_CRYP
    bsp_cryp_init();
#endif
#ifdef BSP_USE_MAC
    bsp_mac_init();
#endif
#ifdef BSP_USE_FSMC
    bsp_fsmc_init();
#endif
#ifdef BSP_USE_DBG
    bsp_dbg_init();
#endif

    return bsp_console_init("uart1");
}


#ifdef __cplusplus
    }
#endif

#endif
