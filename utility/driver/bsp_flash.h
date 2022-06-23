/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_flash.h                                                      */
/* 内容摘要: 主芯片Flash操作头文件                                            */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef BSP_FLASH_H
#define BSP_FLASH_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_device.h"

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
#define BSP_MCU_STM32F4
//#define BSP_MCU_STM32F1

#define _KB                         (1024)
#define STMF4_SECTOR0_SIZE          (_KB * 16)
#define STMF4_SECTOR1_SIZE          (_KB * 16)
#define STMF4_SECTOR2_SIZE          (_KB * 16)
#define STMF4_SECTOR3_SIZE          (_KB * 16)
#define STMF4_SECTOR4_SIZE          (_KB * 64)
#define STMF4_SECTOR5_SIZE          (_KB * 128)
#define STMF4_SECTOR6_SIZE          (_KB * 128)
#define STMF4_SECTOR7_SIZE          (_KB * 128)
#define STMF4_SECTOR8_SIZE          (_KB * 128)
#define STMF4_SECTOR9_SIZE          (_KB * 128)
#define STMF4_SECTOR10_SIZE         (_KB * 128)
#define STMF4_SECTOR11_SIZE         (_KB * 128)
#define STMF4_SECTOR12_SIZE         (_KB * 16)
#define STMF4_SECTOR13_SIZE         (_KB * 16)
#define STMF4_SECTOR14_SIZE         (_KB * 16)
#define STMF4_SECTOR15_SIZE         (_KB * 16)
#define STMF4_SECTOR16_SIZE         (_KB * 64)
#define STMF4_SECTOR17_SIZE         (_KB * 128)
#define STMF4_SECTOR18_SIZE         (_KB * 128)
#define STMF4_SECTOR19_SIZE         (_KB * 128)
#define STMF4_SECTOR20_SIZE         (_KB * 128)
#define STMF4_SECTOR21_SIZE         (_KB * 128)
#define STMF4_SECTOR22_SIZE         (_KB * 128)
#define STMF4_SECTOR23_SIZE         (_KB * 128)
#define STMF4_SYSFLASH_SIZE         (_KB * 30)    /* 系统存储器 */
#define STMF4_OTPAREA_SIZE          (528)
#define STMF4_OPTIONAREA1_SIZE      (16)
#define STMF4_OPTIONAREA2_SIZE      (16)

/* STM32系列各型号处理器Unique_ID存储地址 */
#define SOC_STM32F0_ID_ADDR         (0x1FFFF7AC)
#define SOC_STM32F1_ID_ADDR         (0x1FFFF7E8)
#define SOC_STM32F2_ID_ADDR         (0x1FFF7A10)
#define SOC_STM32F3_ID_ADDR         (0x1FFFF7AC)
#define SOC_STM32F4_ID_ADDR         (0x1FFF7A10)
#define SOC_STM32F7_ID_ADDR         (0x1FF0F420)
#define SOC_STM32L0_ID_ADDR         (0x1FF80050)
#define SOC_STM32L1_ID_ADDR         (0x1FF80050)
#define SOC_STM32L4_ID_ADDR         (0x1FFF7590)
#define SOC_STM32H7_ID_ADDR         (0x1FF0F420)

/* STM32系列各型号处理器CPU_ID统一存储地址 */
#define SOC_STM32CPUID_ADDR         (0xE000ED00)
/* STM32F401xBxCxDxE处理器FLASH大小存储地址 */
#define SOC_STM32FLASH_ADDR         (0x1FFF7A22)
/* STM32F401xBxCxDxE处理器Unique_ID存储地址 */
#define SOC_STM32UNQID_ADDR         (SOC_STM32F4_ID_ADDR)

/* 更换MCU型号时,需要适配调整以下参数 */
#define STM_FLASH_PAGE_SIZE         (_KB * 2)     /* f1由页构成,f4由扇区构成 */
#define STM_FLASH_PAGES             (256)         /* stm32f103 只有256个页   */
#define STM_FLASH_SECTORS           (6)           /* stm32f401 只有6个扇区   */
#define STM_FLASH_ERASE_PAGE        (0x01)        /* 设备操作码之按页擦除    */
#define STM_FLASH_ERASE_SECTOR      (0x02)        /* 设备操作码之按扇区擦除  */

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
#pragma pack(1)

/*<STRUCT+>********************************************************************/
/* 结构: _FLASH_PARA                                                          */
/* 注释: STM32芯片FLASH信息结构体                                            */
/*<STRUCT->********************************************************************/
struct _flash_para
{
    unsigned int flash_start;                        /* FLASH起始地址     */
    unsigned int flash_size;                         /* FLASH总大小       */
    unsigned int flash_end;                          /* FLASH结束地址     */
    unsigned int cpu_ID;                             /* 处理器CPU_ID      */
    unsigned int unique_ID[3];                       /* 处理器Unique_ID   */
    unsigned int pages;                              /* FLASH拆分的总页数 */
    unsigned int page_size;                          /* FLASH的每页大小   */
    unsigned int sectors;                            /* FLASH拆分的总节数 */
};
typedef struct _flash_para *flash_para_t;

/*<STRUCT+>********************************************************************/
/* 结构: FLASH_DEVICE                                                        */
/* 注释: FLASH设备结构体                                                     */
/*<STRUCT->********************************************************************/
struct flash_device
{   
    struct device             dev;                   /* FLASH设备          */
    struct _flash_para        para;                  /* FLASH参数          */
#ifdef  BSP_USE_RT_THREAD
    rt_mutex_t                lock;                  /* FLASH设备锁        */
#endif
};
typedef struct flash_device *flash_device_t;

#pragma pack()

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int bsp_flash_init(const char *name);


#ifdef __cplusplus
    }
#endif

#endif
