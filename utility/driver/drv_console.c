/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: drv_console.c                                                    */
/* 内容摘要: 控制台驱动源文件                                                */
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
#include "drv_console.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  CONSOLE_UART_FIFOSZ   128                   /* CONSOLE外设fifo总大小 */
#define  CONSOLE_UART_BAUD     115200                /* CONSOLE外设串口波特率 */
#define  CONSOLE_UART_WIDTH    UART_WORDLENGTH_8B    /* CONSOLE外设数据位宽   */
#define  CONSOLE_UART_PARITY   UART_PARITY_NONE      /* CONSOLE外设校验方式   */
#define  CONSOLE_UART_STOP     UART_STOPBITS_1       /* CONSOLE外设停止位     */

#ifndef  RT_CONSOLE_DEVICE_NAME
#define  RT_CONSOLE_DEVICE_NAME      "uart1"
#endif

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int  Console_Lock(struct console_device *dev);
static int  Console_Unlock(struct console_device *dev);
#endif

static int _console_init(struct device *dev);
static int _console_open(struct device *dev, int flga);
static int _console_read(struct device *dev, int pos, void *pbuf, int size);
static int _console_write(struct device *dev, int pos, const void *pbuf, int size);
static int _console_rxindicate(struct device *dev, int size);
static int _console_txcomplete(struct device *dev, void *buffer);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
struct console_device *gConsole = NULL;                  /* 设备实体指针    */

static struct _uart_bus_cfg _console_uart_cfg =          /* 控制台串口参数  */
{
    .baud   = CONSOLE_UART_BAUD,
    .width  = CONSOLE_UART_WIDTH,
    .parity = CONSOLE_UART_PARITY,
    .stop   = CONSOLE_UART_STOP,
    .pfifo  = NULL,
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int Console_Lock(struct console_device *dev)
{
    return rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static int Console_Unlock(struct console_device *dev)
{
    return rt_mutex_release(dev->lock);
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: _console_init                                                    */
/* 功能描述: CONSOLE设备操作之初始化                                          */
/* 输入参数: dev --- CONSOLE设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 重设串口外设相关配置参数以及创建fifo                            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _console_init(struct device *dev)
{
    struct console_device *device = (struct console_device *)dev;
    assert_param(device);

    /* 重新配置串口 */
    device->uartx->uart_cfg = &_console_uart_cfg;
    Fn_device_init(&device->uartx->uart_dev);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _console_open                                                    */
/* 功能描述: CONSOLE设备操作之开启                                            */
/* 输入参数: dev --- CONSOLE设备句柄                                          */
/*           flag --- 打开标志                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _console_open(struct device *dev, int flag)
{
    unsigned char *pbuf = NULL;
    struct console_device *device = (struct console_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    pbuf = (unsigned char *)rt_malloc(device->para.fifosz);
#else
    pbuf = (unsigned char *)malloc(device->para.fifosz);
#endif

    assert_param(pbuf);
    memset(pbuf, 0, device->para.fifosz);
    mfifo_init(&device->para.fifo, pbuf, device->para.fifosz);

    /* CONSOLE外设启动(FIFO, rx_indicate, tx_indicate) */
    device->uartx->uart_cfg->pfifo = &device->para.fifo;
    device->uartx->uart_dev.rxind  =  device->dev.rxind;
    device->uartx->uart_dev.txind  =  device->dev.txind;

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _console_read                                                    */
/* 功能描述: CONSOLE设备操作之读取size长数据到pbuf内                         */
/* 输入参数: dev --- CONSOLE设备句柄                                          */
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
static int _console_read(struct device *dev, int pos, void *pbuf, int size)
{
    int    rlen = 0;
    struct console_device *device = (struct console_device *)dev;
    assert_param(device);

    if (size <= 0)  return size;

#ifdef  BSP_USE_RT_THREAD
    Console_Lock(device);
#endif

    rlen = Fn_device_read(&device->uartx->uart_dev, pos, pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    Console_Unlock(device);
#endif

    return (rlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _console_write                                                   */
/* 功能描述: CONSOLE设备操作之发送pbuf指向的size长数据                       */
/* 输入参数: dev --- CONSOLE设备句柄                                          */
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
static int _console_write(struct device *dev, int pos, const void *pbuf, int size)
{
    int    wlen = 0;
    struct console_device *device = (struct console_device *)dev;
    assert_param(device);

    if (size <= 0)  return size;

#ifdef  BSP_USE_RT_THREAD
    Console_Lock(device);
#endif

    wlen = Fn_device_write(&device->uartx->uart_dev, pos, pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    Console_Unlock(device);
#endif

    return (wlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _console_rxindicate                                              */
/* 功能描述: CONSOLE设备操作函数之接收回调                                   */
/* 输入参数: dev --- CONSOLE设备句柄                                          */
/*           size --- 收到的数据长度                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _console_rxindicate(struct device *dev, int size)
{
    struct console_device *device = (struct console_device *)dev->user;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    rt_sem_release(device->rx_sem);
#else

#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _console_txcomplete                                              */
/* 功能描述: CONSOLE设备操作函数之发送完成回调                               */
/* 输入参数: dev --- CONSOLE设备句柄                                          */
/*           buffer --- 已发送完毕的数据区指针                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _console_txcomplete(struct device *dev, void *buffer)
{
    struct console_device *device = (struct console_device *)dev->user;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    rt_sem_release(device->tx_sem);
#else

#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_console_init                                                 */
/* 功能描述: 控制台设备初始化                                                 */
/* 输入参数: name --- 控制台设备名称                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_console_init(const char *name)
{
    struct device         *dev = NULL;
    struct _uart_bus_dev  *bus_dev = NULL;
    struct console_device *device = NULL;

    assert_param(name);
    assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);

#ifdef  BSP_USE_RT_THREAD
    device = (struct console_device *)rt_malloc(sizeof(struct console_device));
#else
    device = (struct console_device *)malloc(sizeof(struct console_device));
#endif

    gConsole = device;
    assert_param(gConsole);
    memset(device, 0, sizeof(struct console_device));

    bus_dev = (struct _uart_bus_dev *)Fn_device_find(RT_CONSOLE_DEVICE_NAME);
    assert_param(bus_dev);
    device->uartx = bus_dev;
    device->uartx->uart_dev.user = device;
    device->para.fifosz = CONSOLE_UART_FIFOSZ;

#ifdef  BSP_USE_RT_THREAD
    struct rt_mutex  *lock = NULL;
    struct rt_semaphore *sem = NULL;

    lock = rt_mutex_create("console_lock", RT_IPC_FLAG_FIFO);
    assert_param(lock);
    device->lock = lock;

    sem = rt_sem_create("console_rxsem", 0, RT_IPC_FLAG_FIFO);
    assert_param(sem);
    device->rx_sem = sem;

    sem = rt_sem_create("console_txsem", 0, RT_IPC_FLAG_FIFO);
    assert_param(sem);
    device->tx_sem = sem;
#endif

    dev = &device->dev;
    strcpy(dev->name, name);

    dev->init  = _console_init;
    dev->open  = _console_open;
    dev->read  = _console_read;
    dev->write = _console_write;
    dev->rxind = _console_rxindicate;
    dev->txind = _console_txcomplete;

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

int _bsp_console_init(void)
{
    return bsp_console_init("console");
}
INIT_DEVICE_EXPORT(_bsp_console_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_console_havec                                                */
/* 功能描述: 控制台是否有输入数据                                            */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_console_havec(void)
{
    return mfifo_have(&gConsole->para.fifo);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_console_getc                                                 */
/* 功能描述: 控制台输入函数                                                  */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: char                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 以阻塞的方式等待串口输入,无数据时会卡在此处                    */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
char bsp_console_getc(void)
{
    unsigned char ch;

    while (Fn_device_read(&gConsole->dev, 0, &ch, 1) != 0)   /* loop wait */
    {
#ifdef  BSP_USE_RT_THREAD
        rt_sem_control(gConsole->rx_sem, RT_IPC_CMD_RESET, NULL);
        rt_sem_take(gConsole->rx_sem, RT_WAITING_FOREVER);
#endif
    }

    return ch;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_console_putc                                                 */
/* 功能描述: 控制台输出字节数据函数                                          */
/* 输入参数: c --- 输出字节数据                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void bsp_console_putc(const unsigned char c)
{
    Fn_device_write(&gConsole->dev, 0, &c, 1);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_console_puts                                                 */
/* 功能描述: 控制台输出字符串函数(对接Fn_mvsprintf.c)                        */
/* 输入参数: str --- 输出字符串指针                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void bsp_console_puts(const char *str)
{
    unsigned char *s = (unsigned char *)str;

    while (*s)
    {
        bsp_console_putc(*s++);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_console_put                                                  */
/* 功能描述: 控制台输出函数                                                  */
/* 输入参数: hex --- 输出数据指针                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void bsp_console_put(const unsigned char *hex, int len)
{
    unsigned char *c = (unsigned char *)hex;

    while (len--)
    {
        bsp_console_putc(*c++);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: rt_hw_console_output                                             */
/* 功能描述: 控制台输出函数,对接shell                                        */
/* 输入参数: str --- 输出字符串指针                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-15              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void rt_hw_console_output(const char *str)
{
    unsigned char c, *s = (unsigned char *)str;

    while (*s)
    {
        if ((*s == '\n') && (c != '\r'))
        {
            bsp_console_putc('\r');
        }
        
        c = *s;
        bsp_console_putc(*s++);
    }

    return;
}

