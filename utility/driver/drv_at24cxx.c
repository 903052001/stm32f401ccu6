/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                             */
/* 文件名称: drv_at24cxx.c                                                    */
/* 内容摘要: AT24Cxx芯片驱动源文件                                            */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-03-27                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-03-27        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "drv_at24cxx.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  AT24C01                   (1 << 7)
#define  AT24C02                   (1 << 8)
#define  AT24C04                   (1 << 9)
#define  AT24C08                   (1 << 10)
#define  AT24C16                   (1 << 11)
#define  AT24C32                   (1 << 12)
#define  AT24C64                   (1 << 13)
#define  AT24C128                  (1 << 14)
#define  AT24C256                  (1 << 15)

#define  AT24CxxTYPE               (AT24C256)
#define  AT24CxxADDR               (0x50)            /* 移位前设备地址 */

#define  DEVICE_NAME               "at24c256"
#define  DRIVER_NAME               "i2c1"

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int  AT24Cxx_Lock(struct at24cxx_device *dev);
static int  AT24Cxx_Unlock(struct at24cxx_device *dev);
#endif

static int  BSP_AT24Cxx_Transfer(struct at24cxx_device *dev, struct _i2c_bus_msg *msgs, int num);
static int  BSP_AT24Cxx_GetID(struct at24cxx_device *dev);
static int  BSP_AT24Cxx_Read(struct at24cxx_device *dev, int rdAddr, unsigned char *rdBuf, int rdSize);
static int  BSP_AT24Cxx_PageWrite(struct at24cxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize);
static int  BSP_AT24Cxx_Write(struct at24cxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize);

static int _at24cxx_init(struct device *dev);
static int _at24cxx_open(struct device *dev, int flga);
static int _at24cxx_close(struct device *dev);
static int _at24cxx_read(struct device *dev, int pos, void *pbuf, int size);
static int _at24cxx_write(struct device *dev, int pos, const void *pbuf, int size);
static int _at24cxx_control(struct device *dev, int cmd, void *args);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static struct at24cxx_device *gAT24Cxx = NULL;               /* 设备指针 */

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int AT24Cxx_Lock(struct at24cxx_device *dev)
{
    return rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static int AT24Cxx_Unlock(struct at24cxx_device *dev)
{
    return rt_mutex_release(dev->lock);
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_AT24Cxx_Transfer                                             */
/* 功能描述: AT24Cxx数据传输函数                                              */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/*           msgs --- I2C数据传输结构体指针                                   */
/*           num --- I2C数据传输结构体个数                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_AT24Cxx_Transfer(struct at24cxx_device *dev, struct _i2c_bus_msg *msgs, int num)
{   
    return (Fn_device_xfer(&dev->i2cx->i2c_dev, msgs, num) > 0) ? 1 : 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_AT24Cxx_GetID                                                */
/* 功能描述: 获取AT24Cxx的设备ID                                              */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_AT24Cxx_GetID(struct at24cxx_device *dev)
{
    unsigned int  i;
    unsigned char aa[256] = {0}, bb[256] = {0};

    for (i = 0; i < 256; i++)
    {
        aa[i] = i;
    }

    BSP_AT24Cxx_Write(dev, 0, aa, 10);
    BSP_AT24Cxx_Read(dev, 0, bb, 10);

    for (i = 0; i < 256; i++)
    {
        //rt_kprintf("%02x ", bb[i]);
    }
    
    return AT24CxxTYPE;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_AT24Cxx_Read                                                 */
/* 功能描述: 实现从AT24Cxx任意指定地址开始读取任意指定长度的数据             */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/*           rdAddr --- 读取地址                                              */
/*           rdBuf --- 指向输出数据的指针                                     */
/*           rdSize --- 读取长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 设备地址中不用的A2,A1,A0全部接地设为0                           */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_AT24Cxx_Read(struct at24cxx_device *dev, int rdAddr, unsigned char *rdBuf, int rdSize)
{
    int ret;
    unsigned char word_addr[2] = {0};
    struct _i2c_bus_msg msg[2] = {0};

    /**************************************************************************/
    /* 输入参数检测                                                           */
    /**************************************************************************/
    assert_param(dev);
    assert_param(rdBuf);
    assert_param(rdSize > 0);
    assert_param(rdAddr < dev->para.e2prom_size);

    if (rdAddr + rdSize > dev->para.e2prom_size)
    {
        rdSize = dev->para.e2prom_size - rdAddr;
    }

    /**************************************************************************/
    /* 写入设备地址和数据子地址                                              */
    /**************************************************************************/
    msg[0].addr = dev->para.slave_addr;
    msg[0].flag = BSP_I2C_WR;
    
    switch (dev->para.e2prom_size)
    {
    case AT24C01:
    case AT24C02:
    case AT24C04:
    case AT24C08:
    case AT24C16:
        word_addr[0] = (unsigned char)(rdAddr >> 8);
        word_addr[1] = (unsigned char)(rdAddr & 0xFF);
        msg[0].addr |=  word_addr[0];   /* 0 1 0 1 0 A2 A1 A0 -> 0 1 0 1 0 A2 A1 P0 */
        msg[0].buf   = &word_addr[1];
        msg[0].len   =  1;
        break;

    case AT24C32:
    case AT24C64:
    case AT24C128:
    case AT24C256:
        word_addr[0] = (unsigned char)(rdAddr >> 8);
        word_addr[1] = (unsigned char)(rdAddr & 0xFF);
        msg[0].buf   = &word_addr[0];
        msg[0].len   =  2;
        break;
    }

    /**************************************************************************/
    /* 读取数据,需要重发起始位                                               */
    /**************************************************************************/
    msg[1].addr = dev->para.slave_addr;
    msg[1].flag = BSP_I2C_RD;
    msg[1].buf  = rdBuf;
    msg[1].len  = rdSize;

    /**************************************************************************/
    /* 总线传输结构体数据                                                     */
    /**************************************************************************/
    ret = BSP_AT24Cxx_Transfer(dev, msg, 2);

    return (ret > 0) ? rdSize : 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_AT24Cxx_PageWrite                                            */
/* 功能描述: AT24Cxx数据页写                                                  */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/*           wrAddr --- 写入地址                                              */
/*           wrBuf --- 指向写入数据的指针                                     */
/*           wrSize --- 写入长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 注意页写环回                                                     */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_AT24Cxx_PageWrite(struct at24cxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize)
{
    int ret;
    unsigned char word_addr[2] = {0};
    struct _i2c_bus_msg msg[2] = {0};
    
    /**************************************************************************/
    /* 输入参数检测                                                           */
    /**************************************************************************/
    assert_param(dev);
    assert_param(wrBuf);
    assert_param(wrSize > 0);
    assert_param(wrAddr < dev->para.e2prom_size);
    
    if (wrSize > dev->para.page_size)
    {
        wrSize = dev->para.page_size;
    }

    /**************************************************************************/
    /* 写入设备地址和数据子地址                                              */
    /**************************************************************************/
    msg[0].addr = dev->para.slave_addr;       /* 芯片地址 0 1 0 1 0 A2 A1 A0 */
    msg[0].flag = BSP_I2C_WR;
    
    switch (dev->para.e2prom_size)
    {
    case AT24C01:
    case AT24C02:
    case AT24C04:
    case AT24C08:
    case AT24C16:
        word_addr[0] = (unsigned char)(wrAddr >> 8);
        word_addr[1] = (unsigned char)(wrAddr & 0xFF);
        msg[0].addr |=  word_addr[0];
        msg[0].buf   = &word_addr[1];
        msg[0].len   =  1;
        break;

    case AT24C32:
    case AT24C64:
    case AT24C128:
    case AT24C256:
        word_addr[0] = (unsigned char)(wrAddr >> 8);
        word_addr[1] = (unsigned char)(wrAddr & 0xFF);
        msg[0].buf   = &word_addr[0];
        msg[0].len   =  2;
        break;
    }

    /**************************************************************************/
    /* 写数据,连续写不需要重发起始位                                         */
    /**************************************************************************/
    msg[1].addr = dev->para.slave_addr;
    msg[1].flag = BSP_I2C_WR | BSP_I2C_NOSTART;
    msg[1].buf  = wrBuf;
    msg[1].len  = wrSize;

    /**************************************************************************/
    /* 总线传输结构体数据                                                     */
    /**************************************************************************/
    ret = BSP_AT24Cxx_Transfer(dev, msg, 2);
    ret = (ret > 0) ? wrSize : 0;
    
    return  ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_AT24Cxx_Write                                                */
/* 功能描述: 实现从AT24Cxx任意指定地址开始写入任意指定长度的数据             */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/*           wrAddr --- 写入地址                                              */
/*           wrBuf --- 指向写入数据的指针                                     */
/*           wrSize --- 写入长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: EEPROM的写入不同于FLASH,不需要擦除                              */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_AT24Cxx_Write(struct at24cxx_device *dev, int wrAddr, unsigned char *wrBuf, int wrSize)
{
    int pages = 0;
    int wAddr = 0;
    int wrLen = 0;
    int count = 0;

    /**************************************************************************/
    /* 输入参数检测                                                           */
    /**************************************************************************/
    assert_param(dev);
    assert_param(wrBuf);
    assert_param(wrSize > 0);
    assert_param(wrAddr < dev->para.e2prom_size);

    if (wrAddr + wrSize > dev->para.e2prom_size)
    {
        wrSize = dev->para.e2prom_size - wrAddr;
    }

    /**************************************************************************/
    /* 计算总写入页数和始尾页字节数                                          */
    /**************************************************************************/
    wrLen = dev->para.page_size - (wrAddr % dev->para.page_size);        /* 第一页剩余可写入长度 */

    if (wrSize > wrLen)
    {
        pages  = (wrSize - wrLen) / dev->para.page_size;                 /* 剩余写入的完整页数 */

        pages += ((wrSize - wrLen) % dev->para.page_size != 0) ? 2 : 1;  /* 总页写数,包括写的不完整的页 */
    }
    else
    {
        pages = 1;
        wrLen = wrSize;
    }

    wAddr = wrAddr;

    /**************************************************************************/
    /* 循环页写入                                                             */
    /**************************************************************************/
    do
    {
        if (0 < BSP_AT24Cxx_PageWrite(dev, wAddr, wrBuf, wrLen))
        {
            wAddr += wrLen;
            wrBuf += wrLen;
            count += wrLen;
        }
        else
        {
            break;
        }

        wrLen = (--pages <= 1) ? (wrSize - count) : (dev->para.page_size);
    }
    while (pages > 0);

    return count;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _at24cxx_init                                                    */
/* 功能描述: AT24Cxx设备操作之初始化                                          */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _at24cxx_init(struct device *dev)
{    
    struct at24cxx_device *device = (struct at24cxx_device *)dev;
    assert_param(device);

    device->para.slave_addr = AT24CxxADDR;

    switch (AT24CxxTYPE)
    {
    case AT24C01:
        device->para.page_size = 8;
        device->para.e2prom_size = 1 << 7;    /* 128B */
        device->para.addr_width = I2C_MEMADD_SIZE_8BIT;
        device->para.e2prom_holdtime = 5;     /* tLOW = tHIGH = 5us */
        device->para.e2prom_wrcycle = 5;      /* tWR = 5ms */
        break;

    case AT24C02:
        device->para.page_size = 8;
        device->para.e2prom_size = 1 << 8;    /* 256B */
        device->para.addr_width = I2C_MEMADD_SIZE_8BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 5;
        break;

    case AT24C04:
        device->para.page_size = 16;
        device->para.e2prom_size = 1 << 9;    /* 512B */
        device->para.addr_width = I2C_MEMADD_SIZE_8BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 5;
        break;

    case AT24C08:
        device->para.page_size = 16;
        device->para.e2prom_size = 1 << 10;    /* 1KB */
        device->para.addr_width = I2C_MEMADD_SIZE_8BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 5;
        break;

    case AT24C16:
        device->para.page_size = 16;
        device->para.e2prom_size = 1 << 11;    /* 2KB */
        device->para.addr_width = I2C_MEMADD_SIZE_8BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 5;
        break;

    case AT24C32:
        device->para.page_size = 32;
        device->para.e2prom_size = 1 << 12;    /* 4KB */
        device->para.addr_width = I2C_MEMADD_SIZE_16BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 10;
        break;

    case AT24C64:
        device->para.page_size = 32;
        device->para.e2prom_size = 1 << 13;    /* 8KB */
        device->para.addr_width = I2C_MEMADD_SIZE_16BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 10;
        break;

    case AT24C128:
        device->para.page_size = 64;
        device->para.e2prom_size = 1 << 14;    /* 16KB */
        device->para.addr_width = I2C_MEMADD_SIZE_16BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 10;
        break;

    case AT24C256:
        device->para.page_size = 64;
        device->para.e2prom_size = 1 << 15;    /* 32KB */
        device->para.addr_width = I2C_MEMADD_SIZE_16BIT;
        device->para.e2prom_holdtime = 5;
        device->para.e2prom_wrcycle = 10;
        break;

    default:
        break;
    }

    device->i2cx->i2c_cfg->holdtime = device->para.e2prom_holdtime;
    device->i2cx->i2c_cfg->wrcycle  = device->para.e2prom_wrcycle;
    
#ifdef  BSP_USE_HARD_I2C
    device->i2cx->i2c_cfg->datawidth = device->para.addr_width;
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _at24cxx_open                                                    */
/* 功能描述: AT24Cxx设备操作之开启                                           */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/*           flag --- 打开标志                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _at24cxx_open(struct device *dev, int flag)
{
    struct at24cxx_device *device = (struct at24cxx_device *)dev;
    assert_param(device);

    BSP_AT24Cxx_GetID(device);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _at24cxx_close                                                   */
/* 功能描述: AT24Cxx设备操作之关闭                                            */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _at24cxx_close(struct device *dev)
{
    struct at24cxx_device *device = (struct at24cxx_device *)dev;
    assert_param(device);

    Fn_device_unregister(&device->dev);

#ifdef  BSP_USE_RT_THREAD
    rt_mutex_delete(device->lock);
    rt_free(device);
#else
    free(device);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _at24cxx_read                                                    */
/* 功能描述: AT24Cxx设备操作之读取size长数据到pbuf内                         */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
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
static int _at24cxx_read(struct device *dev, int pos, void *pbuf, int size)
{
    int    rlen = 0;
    struct at24cxx_device *device = (struct at24cxx_device *)dev;
    assert_param(device);
    
#ifdef  BSP_USE_RT_THREAD
    AT24Cxx_Lock(device);
#endif
    
    rlen = BSP_AT24Cxx_Read(device, pos, (unsigned char *)pbuf, size);
    
#ifdef  BSP_USE_RT_THREAD
    AT24Cxx_Unlock(device);
#endif

    return (rlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _at24cxx_write                                                   */
/* 功能描述: AT24Cxx设备操作之发送pbuf指向的size长数据                       */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
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
static int _at24cxx_write(struct device *dev, int pos, const void *pbuf, int size)
{
    int    wlen = 0;
    struct at24cxx_device *device = (struct at24cxx_device *)dev;
    assert_param(device);
    
#ifdef  BSP_USE_RT_THREAD
    AT24Cxx_Lock(device);
#endif
    
    wlen = BSP_AT24Cxx_Write(device, pos, (unsigned char *)pbuf, size);
    
#ifdef  BSP_USE_RT_THREAD
    AT24Cxx_Unlock(device);
#endif

    return (wlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _at24cxx_control                                                 */
/* 功能描述: AT24Cxx设备操作函数之控制                                        */
/* 输入参数: dev --- AT24Cxx设备句柄                                          */
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
static int _at24cxx_control(struct device *dev, int cmd, void *args)
{
    struct at24cxx_device *device = (struct at24cxx_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    AT24Cxx_Lock(device);
#endif
        
    switch(cmd)
    {
    /* nothing */    
    default:
      break;
    }

#ifdef  BSP_USE_RT_THREAD
    AT24Cxx_Unlock(device);
#endif

    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_at24cxx_init                                                 */
/* 功能描述: AT24Cxx设备初始化和注册函数                                     */
/* 输入参数: device --- AT24Cxx设备句柄                                       */
/*           name --- AT24Cxx设备名称                                         */
/*           bus_name --- SPI总线名称                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_at24cxx_init(const char *name, const char *bus_name)
{
    struct device         *dev = NULL;
    struct _i2c_bus_dev   *bus_dev = NULL;
    struct at24cxx_device *device = NULL;

    assert_param(name);
    assert_param(bus_name);
    assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
    assert_param(sizeof(bus_name) <= DEVICE_NAME_MAXLEN);
    
#ifdef  BSP_USE_RT_THREAD
    device = (struct at24cxx_device *)rt_malloc(sizeof(struct at24cxx_device));
#else
    device = (struct at24cxx_device *)malloc(sizeof(struct at24cxx_device));
#endif

    gAT24Cxx = device;  
    assert_param(gAT24Cxx);
    memset(device, 0, sizeof(struct at24cxx_device));

    bus_dev = (struct _i2c_bus_dev *)Fn_device_find(bus_name);
    assert_param(bus_dev);
    device->i2cx = bus_dev;

#ifdef  BSP_USE_RT_THREAD
    struct rt_mutex  *lock = NULL;

    lock = rt_mutex_create("at24cxx_lock", RT_IPC_FLAG_FIFO);
    assert_param(lock);
    device->lock = lock;
#endif

    dev = &device->dev;
    strcpy(dev->name, name);
    
    dev->init    = _at24cxx_init;
    dev->open    = _at24cxx_open;
    dev->close   = _at24cxx_close;
    dev->read    = _at24cxx_read;
    dev->write   = _at24cxx_write;
    dev->control = _at24cxx_control;

    if(0 != Fn_device_register(dev, name))
    {
        Error_Handler(__FILE__, __LINE__);
    } 

    if(0 != Fn_device_open(dev, 0))
    {
        Error_Handler(__FILE__, __LINE__);
    }

    return 0;
}

int _bsp_at24cxx_init(void)
{
    return bsp_at24cxx_init(DEVICE_NAME, DRIVER_NAME); 
}
INIT_DEVICE_EXPORT(_bsp_at24cxx_init);

