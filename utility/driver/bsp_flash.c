/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_flash.c                                                      */
/* 内容摘要: 主芯片Flash操作源文件                                            */
/* 其它说明: 无                                                               */
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
#include "bsp_flash.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  DEVICE_NAME               "flash"

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int  FLASH_Lock(struct flash_device *dev);
static int  FLASH_Unlock(struct flash_device *dev);
#endif

static int BSP_Flash_ReadData(struct flash_device *dev, unsigned int rAddr, unsigned char *pBuf, int Len);
static int BSP_Flash_WriteData(struct flash_device *dev, unsigned int wAddr, unsigned char *pBuf, int Len);
static int BSP_Flash_ErasePage(struct flash_device *dev, unsigned int BeginAddr, unsigned int EndAddr);
static int BSP_Flash_EraseSector(struct flash_device *dev, int BeginSector, int EndSector);

static int _flash_init(struct device *dev);
static int _flash_open(struct device *dev, int flga);
static int _flash_close(struct device *dev);
static int _flash_read(struct device *dev, int pos, void *pbuf, int size);
static int _flash_write(struct device *dev, int pos, const void *pbuf, int size);
static int _flash_control(struct device *dev, int cmd, void *args);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static struct flash_device *gFlash = NULL;                       /* 设备指针 */

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int FLASH_Lock(struct flash_device *dev)
{
    return rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static int FLASH_Unlock(struct flash_device *dev)
{
    return rt_mutex_release(dev->lock);
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_Flash_ReadData                                               */
/* 功能描述: 从指定地址开始读出数据                                          */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           rAddr --- 读取起始地址                                           */
/*           pBuf --- 数据缓存区                                              */
/*           Len --- 待读取长度                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int BSP_Flash_ReadData(struct flash_device *dev, unsigned int rAddr, unsigned char *pBuf, int Len)
{
    int size = Len;
    
    if ((pBuf == NULL) || (Len <= 0))
    {
        return -1;
    }

    if ((rAddr < dev->para.flash_start) || (rAddr + Len > dev->para.flash_end))
    {
        return -2;
    }

    while (size--)
    {
        *pBuf++ = *(volatile unsigned char *)rAddr++;
    }

    return Len;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_Flash_WriteData                                              */
/* 功能描述: 从指定地址开始写入数据                                          */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           wAddr --- 写入起始地址                                           */
/*           pBuf --- 数据缓存区                                              */
/*           Len --- 待写入长度                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 可以用来写OTP区, OTP区域地址范围: 0x1FFF7800~0x1FFF7A0F         */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int BSP_Flash_WriteData(struct flash_device *dev, unsigned int wAddr, unsigned char *pBuf, int Len)
{
    int size = Len;
    
    if ((pBuf == NULL) || (Len <= 0))
    {
        return -1;
    }

    if ((wAddr < dev->para.flash_start) || (wAddr + Len > dev->para.flash_end))
    {
        return -2;
    }

    HAL_FLASH_Unlock();

    do
    {
        if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, wAddr++, *pBuf++))
        {
            break;
        }
    }while (--size);

    HAL_FLASH_Lock();

    return (size == 0) ? Len : (Len - size);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_Flash_ErasePage                                              */
/* 功能描述: 从指定地址开始擦除若干页                                        */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           BeginAddr --- 开始擦除地址                                       */
/*           EndAddr --- 结束擦除地址                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: only for stm32f1xx                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int BSP_Flash_ErasePage(struct flash_device *dev, unsigned int BeginAddr, unsigned int EndAddr)
{
    unsigned int addr;

    if ((BeginAddr < dev->para.flash_start) || (EndAddr > dev->para.flash_end) || (EndAddr <= BeginAddr))
    {
        return -1;
    }

    if ((BeginAddr % dev->para.page_size != 0) || (EndAddr % dev->para.page_size != 0))
    {
        return -2;
    }

    HAL_FLASH_Unlock();

    for (addr = BeginAddr; addr < EndAddr; addr += dev->para.page_size)
    {
#ifdef  BSP_MCU_STM32F1
        FLASH_ErasePage(addr);
#endif
    }

    HAL_FLASH_Lock();

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: BSP_Flash_EraseSector                                            */
/* 功能描述: 从指定地址开始擦除应用程序所占空间                              */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           BeginSector --- 开始擦除的段序号(从0开始计)                      */
/*           EndSector --- 结束擦除的段序号                                   */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int BSP_Flash_EraseSector(struct flash_device *dev, int BeginSector, int EndSector)
{
    int i, n = EndSector - BeginSector + 1;
    
    if ((BeginSector < 0) || (BeginSector >= dev->para.sectors))
    {
        return -1;
    }

    if ((EndSector < 0) || (EndSector >= dev->para.sectors))
    {
        return -2;
    }

    if (BeginSector > EndSector)
    {
        return -3;
    }

    HAL_FLASH_Unlock();

    for (i = 0; i < n; i++)
    {
#ifdef  BSP_MCU_STM32F4
        FLASH_Erase_Sector(BeginSector + i, FLASH_VOLTAGE_RANGE_3);
#endif
    }

    HAL_FLASH_Lock();

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _flash_init                                                      */
/* 功能描述: FLASH设备操作之初始化                                            */
/* 输入参数: dev --- FLASH设备句柄                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: stm32芯片FLASH信息获取                                           */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _flash_init(struct device *dev)
{
    int i;
    struct flash_device *device = (struct flash_device *)dev;
    assert_param(device);
    
    device->para.flash_start = 0x08000000;
    device->para.flash_size = *(unsigned int *)SOC_STM32FLASH_ADDR & 0xFFFF;
    device->para.flash_end = device->para.flash_size * _KB + device->para.flash_start;
    device->para.cpu_ID = *(unsigned int *)SOC_STM32CPUID_ADDR;
    device->para.pages = STM_FLASH_PAGES;
    device->para.page_size = STM_FLASH_PAGE_SIZE;
    device->para.sectors = STM_FLASH_SECTORS;

    switch ((device->para.cpu_ID >> 4) & 0xFFF)
    {
    case 0xC20:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32F0_ID_ADDR + (i * 4));
        }
        break;

    case 0xC21:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32F1_ID_ADDR + (i * 4));
        }
        break;

    case 0xC23:
        switch ((device->para.cpu_ID >> 16) & 0xFFF)
        {
        case 0x12F:
            for (i = 0; i < 3; i++)
            {
                device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32F2_ID_ADDR + (i * 4));
            }
            break;

        case 0x11F:
            for (i = 0; i < 3; i++)
            {
                device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32F3_ID_ADDR + (i * 4));
            }
            break;

        default:
            break;
        }
        break;

    case 0xC24:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32F4_ID_ADDR + (i * 4));
        }
        break;

    case 0xC27:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32F7_ID_ADDR + (i * 4));
        }
        break;

    case 0xC60:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32L0_ID_ADDR + (i * 4));
        }
        break;

    case 0xC61:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32L1_ID_ADDR + (i * 4));
        }
        break;
        
    case 0xC64:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32L4_ID_ADDR + (i * 4));
        }
        break;
        
    case 0xC67:
        for (i = 0; i < 3; i++)
        {
            device->para.unique_ID[i] = *(unsigned int *)(SOC_STM32H7_ID_ADDR + (i * 4));
        }
        break;
        
    default:
        break;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _flash_open                                                      */
/* 功能描述: FLASH设备操作之开启                                              */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           flag --- 打开标志                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _flash_open(struct device *dev, int flag)
{
    struct flash_device *device = (struct flash_device *)dev;
    assert_param(device);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _flash_close                                                     */
/* 功能描述: FLASH设备操作之关闭                                              */
/* 输入参数: dev --- FLASH设备句柄                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _flash_close(struct device *dev)
{
    struct flash_device *device = (struct flash_device *)dev;
    assert_param(device);

    Fn_device_unregister(&device->dev);    /* 脱离设备列表 */

#ifdef  BSP_USE_RT_THREAD
    rt_mutex_delete(device->lock);
    rt_free(device);
#else
    free(device);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _flash_read                                                      */
/* 功能描述: FLASH设备操作之读取size长数据到pbuf内                           */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           pos --- 从设备那里读取                                           */
/*           pbuf --- 数据接收区                                              */
/*           size --- 读取长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _flash_read(struct device *dev, int pos, void *pbuf, int size)
{
    int    rlen = 0;
    struct flash_device *device = (struct flash_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    FLASH_Lock(device);
#endif

    rlen = BSP_Flash_ReadData(device, pos, (unsigned char *)pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    FLASH_Unlock(device);
#endif

    return (rlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _flash_write                                                     */
/* 功能描述: FLASH设备操作之发送pbuf指向的size长数据                         */
/* 输入参数: dev --- FLASH设备句柄                                            */
/*           pos --- 写入到设备哪里                                           */
/*           pbuf --- 发送数据区                                              */
/*           size --- 发送长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _flash_write(struct device *dev, int pos, const void *pbuf, int size)
{
    int    wlen = 0;
    struct flash_device *device = (struct flash_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    FLASH_Lock(device);
#endif

    wlen = BSP_Flash_WriteData(device, pos, (unsigned char *)pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    FLASH_Unlock(device);
#endif

    return (wlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _flash_control                                                   */
/* 功能描述: FLASH设备操作函数之控制                                         */
/* 输入参数: dev --- FLASH设备句柄                                            */
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
static int _flash_control(struct device *dev, int cmd, void *args)
{
    unsigned int *earse_addr = (unsigned int *)args;
    struct flash_device *device = (struct flash_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    FLASH_Lock(device);
#endif

    switch (cmd)
    {
    case STM_FLASH_ERASE_PAGE:
        BSP_Flash_ErasePage(device, earse_addr[0], earse_addr[1]);
        break;
    
    case STM_FLASH_ERASE_SECTOR:
        BSP_Flash_EraseSector(device, earse_addr[0], earse_addr[1]);
        break;

    default:
        break;
    }

#ifdef  BSP_USE_RT_THREAD
    FLASH_Unlock(device);
#endif

    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_flash_init                                                   */
/* 功能描述: FLASH设备初始化和注册函数                                       */
/* 输入参数: device --- FLASH设备句柄                                         */
/*           name --- FLASH设备名称                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_flash_init(const char *name)
{
    struct device       *dev = NULL;
    struct flash_device *device = NULL;

    assert_param(name);
    assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);

#ifdef  BSP_USE_RT_THREAD
    device = (struct flash_device *)rt_malloc(sizeof(struct flash_device));
#else
    device = (struct flash_device *)malloc(sizeof(struct flash_device));
#endif

    gFlash = device;  
    assert_param(gFlash);
    memset(device, 0, sizeof(struct flash_device));

#ifdef  BSP_USE_RT_THREAD
    struct rt_mutex  *lock = NULL;

    lock = rt_mutex_create("flash_lock", RT_IPC_FLAG_FIFO);
    assert_param(lock);
    device->lock = lock;
#endif

    dev = &device->dev;
    strcpy(dev->name, name);

    dev->init    = _flash_init;
    dev->open    = _flash_open;
    dev->close   = _flash_close;
    dev->read    = _flash_read;
    dev->write   = _flash_write;
    dev->control = _flash_control;

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

int _bsp_flash_init(void)
{
    return bsp_flash_init(DEVICE_NAME); 
}
INIT_BOARD_EXPORT(_bsp_flash_init);
