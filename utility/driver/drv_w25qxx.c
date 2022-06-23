/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: drv_w25qxx.c                                                     */
/* 内容摘要: W25Qxx系列SPI接口FLASH芯片的驱动实现                            */
/* 其它说明: 兼容W25Qxx和GD25Qxx(不必考虑大小端序,直接存/读)                 */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "drv_w25qxx.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
/******************************************************************************/
/* W25QXX/GD25QXX的命令宏定义                                                 */
/******************************************************************************/
#define CMD_PP                     (0x02)   /* Page Program                  */
#define CMD_READ                   (0x03)   /* Read Data                     */
#define CMD_FAST_READ              (0x0B)   /* Fast Read                     */
#define CMD_WRDI                   (0x04)   /* Write Disable                 */
#define CMD_WREN                   (0x06)   /* Write Enable                  */
#define CMD_RDCR                   (0x15)   /* Read Config Register          */
#define CMD_WRSR                   (0x01)   /* Write Status Register         */
#define CMD_RDSR                   (0x05)   /* Read Status Register          */
#define CMD_RDSR1                  (0x05)   /* Read Status Register-1        */
#define CMD_RDSR2                  (0x35)   /* Read Status Register-2        */
#define CMD_ERASE_4K               (0x20)   /* Sector Erase(4KB)             */
#define CMD_ERASE_32K              (0x52)   /* 32KB Block Erase              */
#define CMD_ERASE_64K              (0xD8)   /* 64KB Block Erase              */
#define CMD_ERASE_CHIP             (0x60)   /* Chip Erase                    */
#define CMD_JEDEC_ID               (0x9F)   /* Read JEDEC ID                 */
#define CMD_Power_Down             (0xB9)   /* Power Down                    */
#define CMD_Release_PD             (0xAB)   /* Release Power Down            */

/******************************************************************************/
/* W25QXX/GD25QXX内部寄存器位宏定义                                           */
/******************************************************************************/
#define SR_WIP_POS                 (0)
#define SR_WIP_MSK                 (1 << SR_WIP_POS)
#define SR_WIP                     (SR_WIP_MSK)

#define SR_WEL_POS                 (1)
#define SR_WEL_MSK                 (1 << SR_WEL_POS)
#define SR_WEL                     (SR_WEL_MSK)

#define SR_BP0_POS                 (2)
#define SR_BP0_MSK                 (1 << SR_BP0_POS)
#define SR_BP0                     (SR_BP0_MSK)

#define SR_BP1_POS                 (3)
#define SR_BP1_MSK                 (1 << SR_BP1_POS)
#define SR_BP1                     (SR_BP1_MSK)

#define SR_BP2_POS                 (4)
#define SR_BP2_MSK                 (1 << SR_BP2_POS)
#define SR_BP2                     (SR_BP2_MSK)

#define SR_BP3_POS                 (5)
#define SR_BP3_MSK                 (1 << SR_BP2_POS)
#define SR_BP3                     (SR_BP2_MSK)

#define SR_BP4_POS                 (6)
#define SR_BP4_MSK                 (1 << SR_BP2_POS)
#define SR_BP4                     (SR_BP2_MSK)

#define SR_SRP0_POS                (7)
#define SR_SRP0_MSK                (1 << SR_SRP0_POS)
#define SR_SRP0                    (SR_SRP0_MSK)

#define SR_SRP1_POS                (8)
#define SR_SRP1_MSK                (1 << SR_SRP1_POS)
#define SR_SRP1                    (SR_SRP1_MSK)

#define SR_QE_POS                  (9)
#define SR_QE_MSK                  (1 << SR_QE_POS)
#define SR_QE                      (SR_QE_MSK)

#define SR_SUS2_POS                (10)
#define SR_SUS2_MSK                (1 << SR_SUS2_POS)
#define SR_SUS2                    (SR_SUS2_MSK)

#define SR_LB1_POS                 (11)
#define SR_LB1_MSK                 (1 << SR_LB1_POS)
#define SR_LB1                     (SR_LB1_MSK)

#define SR_LB2_POS                 (12)
#define SR_LB2_MSK                 (1 << SR_LB2_POS)
#define SR_LB2                     (SR_LB2_MSK)

#define SR_LB3_POS                 (13)
#define SR_LB3_MSK                 (1 << SR_LB3_POS)
#define SR_LB3                     (SR_LB3_MSK)

#define SR_CMP_POS                 (14)
#define SR_CMP_MSK                 (1 << SR_CMP_POS)
#define SR_CMP                     (SR_CMP_MSK)

#define SR_SUS1_POS                (15)
#define SR_SUS1_MSK                (1 << SR_SUS1_POS)
#define SR_SUS1                    (SR_SUS1_MSK)

#define SR_HPF_POS                 (20)
#define SR_HPF_MSK                 (1 << SR_HPF_POS)
#define SR_HPF                     (SR_HPF_MSK)

#define SR_DRV0_POS                (21)
#define SR_DRV0_MSK                (1 << SR_DRV0_POS)
#define SR_DRV0                    (SR_DRV0_MSK)

#define SR_DRV1_POS                (22)
#define SR_DRV1_MSK                (1 << SR_DRV1_POS)
#define SR_DRV1                    (SR_DRV1_MSK)

/******************************************************************************/
/* W25Qxx芯片ID                                                               */
/******************************************************************************/
#define W25QXX_MF_ID               (0xEF)      /* manufacturer-id */
#define W25Q40_BV_BL               (0xEF4013)  /* 512KB: W25Q40BV W25Q40BL */
#define W25Q80_BV_BL               (0xEF4014)  /* 1MB:   W25Q80BV W25Q80BL */
#define W25Q16_BV_CL_CV            (0xEF4015)  /* 2MB:   W25Q16BV W25Q16CL W25Q16CV */
#define W25Q16_DW                  (0xEF6015)  /* 2MB:   W25Q16DW */
#define W25Q32_BV                  (0xEF4016)  /* 4MB:   W25Q32BV */
#define W25Q32_DW                  (0xEF6016)  /* 4MB:   W25Q32DW */
#define W25Q64_BV_CV               (0xEF4017)  /* 8MB:   W25Q64BV W25Q64CV */
#define W25Q64_DW                  (0xEF6017)  /* 8MB:   W25Q64DW */
#define W25Q128_BV                 (0xEF4018)  /* 16MB:  W25Q128BV */
#define W25Q256_FV_S               (0xEF4019)  /* 32MB:  W25Q256FV SPI mode */
#define W25Q256_FV_Q               (0xEF6019)  /* 32MB:  W25Q256FV QPI mode */

/******************************************************************************/
/* GD25Qxx芯片ID                                                              */
/******************************************************************************/
#define GD25QXX_MF_ID              (0xC8)      /* manufacturer-id */
#define GD25Q10_ID                 (0xC84011)  /* 128KB */
#define GD25Q20_ID                 (0xC84012)  /* 256KB */
#define GD25Q40_ID                 (0xC84013)  /* 512KB */
#define GD25Q80_ID                 (0xC84014)  /* 1MB */
#define GD25Q16_ID                 (0xC84015)  /* 2MB */
#define GD25Q32_ID                 (0xC84016)  /* 4MB */
#define GD25Q64_ID                 (0xC84017)  /* 8MB */

/******************************************************************************/
/* SPI引脚宏定义: SPI1 CS:PA4 | CLK:PA5 | MISO:PA6 | MOSI:PA7                 */
/******************************************************************************/
#define W25Qxx_CS_PORT              GPIOA
#define W25Qxx_CS_PIN               GPIO_PIN_4

#define DEVICE_NAME                 "w25q64"
#define DRIVER_NAME                 "spi1"

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/
/*<ENUM+>**********************************************************************/
/* 枚举: _W25QXX_CS                                                           */
/* 注释: CS状态枚举                                                           */
/*<ENUM->**********************************************************************/
enum _w25qxx_cs
{
    SELECT = 0,
    UNSELECT = !SELECT,
};

/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int  W25Qxx_Lock(struct w25qxx_device *dev);
static int  W25Qxx_Unlock(struct w25qxx_device *dev);
#endif

static void BSP_W25Qxx_CS(struct w25qxx_device *dev, enum _w25qxx_cs cs);
static void BSP_W25Qxx_WriteEnable(struct w25qxx_device *dev);
static void BSP_W25Qxx_WaitOptFinish(struct w25qxx_device *dev);
static int  BSP_W25Qxx_Transfer(struct w25qxx_device *dev, unsigned char tx);
static int  BSP_W25Qxx_GetID(struct w25qxx_device *dev);
static int  BSP_W25Qxx_ChipErase(struct w25qxx_device *dev);
static int  BSP_W25Qxx_SectorErase(struct w25qxx_device *dev, int sector);
static int  BSP_W25Qxx_Read(struct w25qxx_device *dev, int rdAddr, unsigned char *rdBuf, int rdSize);
static int  BSP_W25Qxx_Write(struct w25qxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize);
static int  BSP_W25Qxx_PageWrite(struct w25qxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize);
static int  BSP_W25Qxx_DataProgram(struct w25qxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize);

static int _w25qxx_init(struct device *dev);
static int _w25qxx_open(struct device *dev, int flga);
static int _w25qxx_close(struct device *dev);
static int _w25qxx_read(struct device *dev, int pos, void *pbuf, int size);
static int _w25qxx_write(struct device *dev, int pos, const void *pbuf, int size);
static int _w25qxx_control(struct device *dev, int cmd, void *args);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static unsigned char *gW25QxxBuf = NULL;                /* 读-改-写缓存区    */
static struct w25qxx_device *gW25Qxx = NULL;            /* 设备指针          */

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int W25Qxx_Lock(struct w25qxx_device *dev)
{
    return rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static int W25Qxx_Unlock(struct w25qxx_device *dev)
{
    return rt_mutex_release(dev->lock);
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_CS                                                    */
/* 功能描述: CS引脚的控制                                                     */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           cs --- CS引脚状态; 0-输出低电平; !0-输出高电平                  */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void BSP_W25Qxx_CS(struct w25qxx_device *dev, enum _w25qxx_cs cs)
{
    if (SELECT == cs)
    {
        HAL_GPIO_WritePin(dev->para.cs_port, dev->para.cs_pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(dev->para.cs_port, dev->para.cs_pin, GPIO_PIN_SET);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_Transfer                                              */
/* 功能描述: W25Qxx发送并接收1字节的数据                                      */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           tx --- 发送的字节数据                                            */
/* 输出参数: 返回收到的1字节数据                                             */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_Transfer(struct w25qxx_device *dev, unsigned char tx)
{
    return Fn_device_xfer(&dev->spix->spi_dev, &tx, 1);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_WriteEnable                                           */
/* 功能描述: 使能写入操作                                                     */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 在进行页编程,扇区擦除,块擦除,全片擦除等操作前应首先使能写入    */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void BSP_W25Qxx_WriteEnable(struct w25qxx_device *dev)
{
    BSP_W25Qxx_CS(dev, SELECT);

    BSP_W25Qxx_Transfer(dev, CMD_WREN);

    BSP_W25Qxx_CS(dev, UNSELECT);

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_WaitOptFinish                                         */
/* 功能描述: 等待对W25Qxx操作的结束                                           */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void BSP_W25Qxx_WaitOptFinish(struct w25qxx_device *dev)
{
    char state;

    BSP_W25Qxx_CS(dev, SELECT);

    BSP_W25Qxx_Transfer(dev, CMD_RDSR);

    do
    {
        state = BSP_W25Qxx_Transfer(dev, 0xFF);
    }
    while (0 != (state & SR_WIP));

    BSP_W25Qxx_CS(dev, UNSELECT);

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_GetID                                                 */
/* 功能描述: 获取W25Qxx的设备ID                                               */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_GetID(struct w25qxx_device *dev)
{
    char  rdByte = 0;
    unsigned int devID  = 0;

    BSP_W25Qxx_CS(dev, SELECT);

    BSP_W25Qxx_Transfer(dev, CMD_JEDEC_ID);

    rdByte = BSP_W25Qxx_Transfer(dev, 0xFF);
    devID |= (unsigned int)rdByte << 16;

    rdByte = BSP_W25Qxx_Transfer(dev, 0xFF);
    devID |= (unsigned int)rdByte << 8;

    rdByte = BSP_W25Qxx_Transfer(dev, 0xFF);
    devID |= (unsigned int)rdByte;

    BSP_W25Qxx_CS(dev, UNSELECT);

    return devID;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_ChipErase                                             */
/* 功能描述: 全片擦除,耗时很长                                                */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_ChipErase(struct w25qxx_device *dev)
{
    BSP_W25Qxx_WriteEnable(dev);

    BSP_W25Qxx_CS(dev, SELECT);
    BSP_W25Qxx_Transfer(dev, CMD_ERASE_CHIP);
    BSP_W25Qxx_CS(dev, UNSELECT);

    BSP_W25Qxx_WaitOptFinish(dev);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_SectorErase                                           */
/* 功能描述: 擦除一个扇区                                                     */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           sector --- 扇区序号(从0开始计数)                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_SectorErase(struct w25qxx_device *dev, int sector)
{
    int secAddr = sector * dev->para.sector_size;

    BSP_W25Qxx_WriteEnable(dev);

    BSP_W25Qxx_CS(dev, SELECT);
    BSP_W25Qxx_Transfer(dev,  CMD_ERASE_4K);
    BSP_W25Qxx_Transfer(dev, (secAddr & 0xFF0000) >> 16);
    BSP_W25Qxx_Transfer(dev, (secAddr & 0x00FF00) >> 8);
    BSP_W25Qxx_Transfer(dev, (secAddr & 0x0000FF));
    BSP_W25Qxx_CS(dev, UNSELECT);

    BSP_W25Qxx_WaitOptFinish(dev);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_Read                                                  */
/* 功能描述: 从W25Qxx指定地址开始读取指定长度的数据                          */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           rdAddr --- 读取接收地址                                          */
/*           rdBuf --- 指向输出数据的指针                                     */
/*           rdSize --- 读取长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_Read(struct w25qxx_device *dev, int rdAddr, unsigned char *rdBuf, int rdSize)
{
    unsigned int  size = 0;
    unsigned char *buf = rdBuf;

    /**************************************************************************/
    /* 输入参数检查                                                           */
    /**************************************************************************/
    assert_param(dev);
    assert_param(rdBuf);
    assert_param(rdSize > 0);
    assert_param(rdAddr < dev->para.flash_size);

    if (rdAddr + rdSize > dev->para.flash_size)
    {
        rdSize = dev->para.flash_size - rdAddr;
    }

    /**************************************************************************/
    /* 读之前写失能,防止误操作                                               */
    /**************************************************************************/
    BSP_W25Qxx_CS(dev, SELECT);
    BSP_W25Qxx_Transfer(dev, CMD_WRDI);
    BSP_W25Qxx_CS(dev, UNSELECT);

    /**************************************************************************/
    /* 读取数据:1.CS低->2.发送读命令->3.发送读地址->4.读数据->5.CS高        */
    /**************************************************************************/
    BSP_W25Qxx_CS(dev, SELECT);

    BSP_W25Qxx_Transfer(dev,  CMD_READ);

    BSP_W25Qxx_Transfer(dev, (rdAddr & 0xFF0000) >> 16);
    BSP_W25Qxx_Transfer(dev, (rdAddr & 0x00FF00) >> 8);
    BSP_W25Qxx_Transfer(dev, (rdAddr & 0x0000FF));

    while (rdSize--)
    {
        *buf++ = BSP_W25Qxx_Transfer(dev, 0xFF);
        size++;
    }

    BSP_W25Qxx_CS(dev, UNSELECT);

    return size;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_PageWrite                                             */
/* 功能描述: 页编程                                                           */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           wrAddr --- 写入地址                                              */
/*           wrBuf --- 指向写入数据的指针                                     */
/*           wrSize --- 写入数据的长度                                        */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 页编程注意地址环回                                              */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_PageWrite(struct w25qxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize)
{
    int cnts;
  
    assert_param(dev);
    assert_param(wrBuf);
    assert_param(wrSize > 0);
    assert_param(wrAddr < dev->para.flash_size);

    if (wrSize > dev->para.page_size)
    {
        wrSize = dev->para.page_size;
    }

    BSP_W25Qxx_WriteEnable(dev);

    BSP_W25Qxx_CS(dev, SELECT);

    BSP_W25Qxx_Transfer(dev,  CMD_PP);
    BSP_W25Qxx_Transfer(dev, (wrAddr & 0xFF0000) >> 16);
    BSP_W25Qxx_Transfer(dev, (wrAddr & 0x00FF00) >> 8);
    BSP_W25Qxx_Transfer(dev, (wrAddr & 0x0000FF));

    for (cnts = 0; cnts < wrSize; cnts++)
    {
        BSP_W25Qxx_Transfer(dev, wrBuf[cnts]);
    }

    BSP_W25Qxx_CS(dev, UNSELECT);

    BSP_W25Qxx_WaitOptFinish(dev);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_DataProgram                                           */
/* 功能描述: 数据编程                                                         */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           wrAddr --- 写入地址                                              */
/*           wrBuf --- 要写入的数据                                           */
/*           wrSize --- 数据长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 支持从任意地址处写入任意长数据(前提该区域已擦过)               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_DataProgram(struct w25qxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize)
{
    int pageRemain;
    unsigned char *pData = wrBuf;

    /**************************************************************************/
    /* 输入参数检查                                                           */
    /**************************************************************************/
    assert_param(dev);
    assert_param(wrBuf);
    assert_param(wrSize > 0);
    assert_param(wrAddr < dev->para.flash_size);

    if (wrAddr + wrSize > dev->para.flash_size)
    {
        wrSize = dev->para.flash_size - wrAddr;
    }

    /**************************************************************************/
    /* 计算写入地址所在的页还剩多少写入空间                                  */
    /**************************************************************************/
    pageRemain = dev->para.page_size - (wrAddr % dev->para.page_size);

    if (wrSize <= pageRemain)
    {
        pageRemain = wrSize;
    }

    /**************************************************************************/
    /* 以页为单位进行写入                                                     */
    /**************************************************************************/
    while (1)
    {
        BSP_W25Qxx_PageWrite(dev, wrAddr, pData, pageRemain);

        if (pageRemain == wrSize)
        {
            break;
        }
        else
        {
            pData  += pageRemain;
            wrAddr += pageRemain;
            wrSize -= pageRemain;

            if (wrSize > dev->para.page_size)
            {
                pageRemain = dev->para.page_size;
            }
            else
            {
                pageRemain = wrSize;
            }
        }
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_W25Qxx_Write                                                 */
/* 功能描述: 数据写入                                                         */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           wrAddr --- 写入地址                                              */
/*           wrBuf --- 指向写入数据的指针                                     */
/*           wrSize --- 写入长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 支持从任意地址处写入任意长数据(段操作,读-改-写)                */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_W25Qxx_Write(struct w25qxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize)
{
    int secPos = 0;
    int secOffset = 0;
    int secRemainSize = 0;
    int cnts = 0;
    int cunt = 0;
    
    /**************************************************************************/
    /* 输入参数检查                                                           */
    /**************************************************************************/
    assert_param(dev);
    assert_param(wrBuf);
    assert_param(wrSize > 0);
    assert_param(wrAddr < dev->para.flash_size);

    if (wrAddr + wrSize > dev->para.flash_size)
    {
        wrSize = dev->para.flash_size - wrAddr;
    }

    /**************************************************************************/
    /* 依据写入地址和写入长度，计算相关的写入信息                           */
    /**************************************************************************/
    secPos    = wrAddr / dev->para.sector_size;
    secOffset = wrAddr % dev->para.sector_size;

    secRemainSize = dev->para.sector_size - secOffset;
    if (wrSize <= secRemainSize)
    {
        secRemainSize = wrSize;
    }

#ifdef  BSP_USE_RT_THREAD
    gW25QxxBuf = (unsigned char *)rt_malloc(dev->para.sector_size);
#else
    gW25QxxBuf = (unsigned char *)malloc(dev->para.sector_size);
#endif
    assert_param(gW25QxxBuf != NULL);

    /**************************************************************************/
    /* 循环写入数据                                                           */
    /**************************************************************************/
    while (1)
    {
        /**********************************************************************/
        /* 读出整个扇区的数据                                                 */
        /**********************************************************************/
        BSP_W25Qxx_Read(dev, secPos * dev->para.sector_size, gW25QxxBuf, dev->para.sector_size);

        /**********************************************************************/
        /* 判断写入数据的扇区是否需要擦除;如果需要则先擦后写;否则直接写入   */
        /**********************************************************************/
        for (cnts = 0; cnts < secRemainSize; cnts++)
        {
            if (0xFF != (unsigned char)gW25QxxBuf[secOffset + cnts])
            {
                break;
            }
        }

        if (cnts < secRemainSize)
        {
            BSP_W25Qxx_SectorErase(dev, secPos);

            for (cnts = 0; cnts < secRemainSize; cnts++)
            {
                gW25QxxBuf[secOffset + cnts] = wrBuf[cnts];
            }

            BSP_W25Qxx_DataProgram(dev, secPos * dev->para.sector_size, gW25QxxBuf, dev->para.sector_size);
        }
        else
        {
            BSP_W25Qxx_DataProgram(dev, wrAddr, wrBuf, secRemainSize);
        }

        /**********************************************************************/
        /* 记录写入数据总数                                                  */
        /**********************************************************************/
        cunt += secRemainSize;

        /**********************************************************************/
        /* 数据写入结束                                                       */
        /**********************************************************************/
        if (wrSize == secRemainSize)
        {
            break;
        }
        /**********************************************************************/
        /* 调整参数，准备下一次写入                                          */
        /**********************************************************************/
        else
        {
            secPos++;
            secOffset = 0;

            wrBuf  += secRemainSize;
            wrAddr += secRemainSize;
            wrSize -= secRemainSize;

            if (wrSize > dev->para.sector_size)
            {
                secRemainSize = dev->para.sector_size;
            }
            else
            {
                secRemainSize = wrSize;
            }
        }
    }

#ifdef  BSP_USE_RT_THREAD
    if (NULL != gW25QxxBuf)  rt_free(gW25QxxBuf);
#else
    if (NULL != gW25QxxBuf)  free(gW25QxxBuf);
#endif

    return cunt;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _w25qxx_init                                                     */
/* 功能描述: W25Qxx设备操作之初始化                                           */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _w25qxx_init(struct device *dev)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    struct w25qxx_device *device = (struct w25qxx_device *)dev;
    assert_param(device);

    device->para.cs_port = W25Qxx_CS_PORT;
    device->para.cs_pin = W25Qxx_CS_PIN;
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure CS_Pin */
    GPIO_InitStruct.Pin = device->para.cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(device->para.cs_port, &GPIO_InitStruct);

    BSP_W25Qxx_CS(device, UNSELECT);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _w25qxx_open                                                     */
/* 功能描述: W25Qxx设备操作之开启                                            */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           flag --- 打开标志                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _w25qxx_open(struct device *dev, int flag)
{
    int ret = 0;
    struct w25qxx_device *device = (struct w25qxx_device *)dev;
    assert_param(device);

    switch (BSP_W25Qxx_GetID(device))
    {
    case GD25Q10_ID:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 17;   /* 128KB */
        break;

    case GD25Q20_ID:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 18;   /* 256KB */
        ret = 2;
        break;

    case GD25Q40_ID:
    case W25Q40_BV_BL:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 19;   /* 512KB */
        break;

    case GD25Q80_ID:
    case W25Q80_BV_BL:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 20;   /* 1MB */
        break;

    case GD25Q16_ID:
    case W25Q16_DW:
    case W25Q16_BV_CL_CV:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 21;   /* 2MB */
        break;

    case GD25Q32_ID:
    case W25Q32_BV:
    case W25Q32_DW:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 22;   /* 4MB */
        break;

    case GD25Q64_ID:
    case W25Q64_BV_CV:
    case W25Q64_DW:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 23;   /* 8MB */
        break;

    case W25Q128_BV:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 24;   /* 16MB */
        break;

    case W25Q256_FV_S:
    case W25Q256_FV_Q:
        device->para.page_size   = 1 << 8;    /* 256B */
        device->para.sector_size = 1 << 12;   /* 4KB */
        device->para.flash_size  = 1 << 25;   /* 32MB */
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _w25qxx_close                                                    */
/* 功能描述: W25Qxx设备操作之关闭                                             */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _w25qxx_close(struct device *dev)
{
    struct w25qxx_device *device = (struct w25qxx_device *)dev;
    assert_param(device);

    Fn_device_unregister(&device->dev);    /* 脱离设备列表 */

    HAL_GPIO_DeInit(device->para.cs_port, device->para.cs_pin);

#ifdef  BSP_USE_RT_THREAD
    rt_mutex_delete(device->lock);
    rt_free(device);
#else
    free(device);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _w25qxx_read                                                     */
/* 功能描述: W25Qxx设备操作之读取size长数据到pbuf内                          */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           pos --- 从设备那里读取                                           */
/*           pbuf --- 数据接收区                                              */
/*           size --- 读取长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 欲读取的长度与实际读取的长度相等时返回0, 否则返回-1            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _w25qxx_read(struct device *dev, int pos, void *pbuf, int size)
{
    int    rlen = 0;
    struct w25qxx_device *device = (struct w25qxx_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    W25Qxx_Lock(device);
#endif

    rlen = BSP_W25Qxx_Read(device, pos, (unsigned char *)pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    W25Qxx_Unlock(device);
#endif

    return (rlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _w25qxx_write                                                    */
/* 功能描述: W25Qxx设备操作之发送pbuf指向的size长数据                        */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           pos --- 写入到设备哪里                                           */
/*           pbuf --- 发送数据区                                              */
/*           size --- 发送长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 欲写出的长度与实际写出的长度相等时返回0, 否则返回-1            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _w25qxx_write(struct device *dev, int pos, const void *pbuf, int size)
{
    int    wlen = 0;
    struct w25qxx_device *device = (struct w25qxx_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    W25Qxx_Lock(device);
#endif

    wlen = BSP_W25Qxx_Write(device, pos, (unsigned char *)pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    W25Qxx_Unlock(device);
#endif

    return (wlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _w25qxx_control                                                  */
/* 功能描述: W25Qxx设备操作函数之控制                                         */
/* 输入参数: dev --- W25Qxx设备句柄                                           */
/*           cmd --- 控制命令                                                 */
/*           args --- 控制参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _w25qxx_control(struct device *dev, int cmd, void *args)
{
    struct w25qxx_device *device = (struct w25qxx_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    W25Qxx_Lock(device);
#endif

    switch (cmd)
    {
    case W25QXX_FLASH_ERASE_CHIP:
        BSP_W25Qxx_ChipErase(device);
        break;
    
    case W25QXX_FLASH_ERASE_SECTOR:
        BSP_W25Qxx_SectorErase(device, *(int *)args);
        break;

    default:
        break;
    }

#ifdef  BSP_USE_RT_THREAD
    W25Qxx_Unlock(device);
#endif

    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_w25qxx_init                                                  */
/* 功能描述: W25Qxx设备初始化和注册函数                                       */
/* 输入参数: device --- W25Qxx设备句柄                                        */
/*           name --- W25Qxx设备名称                                          */
/*           bus_name --- SPI总线名称                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_w25qxx_init(const char *name, const char *bus_name)
{
    struct device        *dev = NULL;
    struct _spi_bus_dev  *bus_dev = NULL;
    struct w25qxx_device *device = NULL;

    assert_param(name);
    assert_param(bus_name);
    assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
    assert_param(sizeof(bus_name) <= DEVICE_NAME_MAXLEN);

#ifdef  BSP_USE_RT_THREAD
    device = (struct w25qxx_device *)rt_malloc(sizeof(struct w25qxx_device));
#else
    device = (struct w25qxx_device *)malloc(sizeof(struct w25qxx_device));
#endif

    gW25Qxx = device;  
    assert_param(gW25Qxx);
    memset(device, 0, sizeof(struct w25qxx_device));

    bus_dev = (struct _spi_bus_dev *)Fn_device_find(bus_name);
    assert_param(bus_dev);
    device->spix = bus_dev;

    device->para.cs_port = W25Qxx_CS_PORT;
    device->para.cs_pin  = W25Qxx_CS_PIN;

#ifdef  BSP_USE_RT_THREAD
    struct rt_mutex  *lock = NULL;

    lock = rt_mutex_create("w25qxx_lock", RT_IPC_FLAG_FIFO);
    assert_param(lock);
    device->lock = lock;
#endif

    dev = &device->dev;
    strcpy(dev->name, name);

    dev->init    = _w25qxx_init;
    dev->open    = _w25qxx_open;
    dev->close   = _w25qxx_close;
    dev->read    = _w25qxx_read;
    dev->write   = _w25qxx_write;
    dev->control = _w25qxx_control;

    if (0 != Fn_device_register(dev, name))
    {
        Error_Handler(__FILE__, __LINE__);
    }

    if (0 != Fn_device_open(dev, 0))
    {
        Error_Handler(__FILE__, __LINE__);
    }

    return 0;
}

int _bsp_w25qxx_init(void)
{
    return bsp_w25qxx_init(DEVICE_NAME, DRIVER_NAME); 
}
INIT_DEVICE_EXPORT(_bsp_w25qxx_init);

