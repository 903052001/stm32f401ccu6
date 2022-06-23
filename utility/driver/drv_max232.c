/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: drv_max232.c                                                     */
/* 内容摘要: MAX232芯片驱动源文件                                             */
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
#include "drv_max232.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  MAX232_UART_FIFOSZ    256                   /* MAX232外设fifo总大小 */
#define  MAX232_UART_BAUD      115200                /* MAX232外设串口波特率 */
#define  MAX232_UART_WIDTH     UART_WORDLENGTH_8B    /* MAX232外设数据位宽   */
#define  MAX232_UART_PARITY    UART_PARITY_NONE      /* MAX232外设校验方式   */
#define  MAX232_UART_STOP      UART_STOPBITS_1       /* MAX232外设停止位     */

#ifdef   MAX232_USE_FLOW_CTRL
#define  MAX232_CTS_PORT       GPIOA                 /* 默认USART2流控脚    */
#define  MAX232_CTS_PIN        GPIO_PIN_0
#define  MAX232_RTS_PORT       GPIOA
#define  MAX232_RTS_PIN        GPIO_PIN_1
#endif

#define  DEVICE_NAME           "MAX232"
#define  DRIVER_NAME           "uart2"

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int  Max232_Lock(struct max232_device *dev);
static int  Max232_Unlock(struct max232_device *dev);
#endif

static void Max232_Parser(void *parameter);
static int _max232_init(struct device *dev);
static int _max232_open(struct device *dev, int flga);
static int _max232_close(struct device *dev);
static int _max232_read(struct device *dev, int pos, void *pbuf, int size);
static int _max232_write(struct device *dev, int pos, const void *pbuf, int size);
static int _max232_control(struct device *dev, int cmd, void *args);
static int _max232_rxindicate(struct device *dev, int size);
static int _max232_txcomplete(struct device *dev, void *buffer);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static struct max232_device *gMax232 = NULL;                     /* 设备指针 */

static struct _uart_bus_cfg _max232_uart_cfg =         /* MAX232外设串口参数 */
{
    .baud   = MAX232_UART_BAUD,
    .width  = MAX232_UART_WIDTH,
    .parity = MAX232_UART_PARITY,
    .stop   = MAX232_UART_STOP,
    .pfifo  = NULL,
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int Max232_Lock(struct max232_device *dev)
{
    return rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static int Max232_Unlock(struct max232_device *dev)
{
    return rt_mutex_release(dev->lock);
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: Max232_Parser                                                    */
/* 功能描述: MAX232数据协议解析器                                             */
/* 输入参数: parameter --- 参数指针                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void Max232_Parser(void *parameter)
{
    unsigned char ch = 0;
    struct max232_device *device = (struct max232_device *)parameter;
    
    while(1)
    {
        while (Fn_device_read(&device->dev, 0, &ch, 1) != 0)
        {
        #ifdef  BSP_USE_RT_THREAD
            rt_sem_control(device->rx_sem, RT_IPC_CMD_RESET, NULL);
            rt_sem_take(device->rx_sem, RT_WAITING_FOREVER);
        #else
            return;
        #endif
        }
    
        Fn_device_write(&device->dev, 0, &ch, 1);
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_init                                                     */
/* 功能描述: MAX232设备操作之初始化                                           */
/* 输入参数: dev --- MAX232设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 重设串口外设相关配置参数以及创建fifo                            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max232_init(struct device *dev)
{
    struct max232_device *device = (struct max232_device *)dev;
    assert_param(device);

    /* 重新配置串口 */
    device->uartx->uart_cfg = &_max232_uart_cfg;
    Fn_device_init(&device->uartx->uart_dev);

#ifdef  MAX232_USE_FLOW_CTRL
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;    /* 流控必须此模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;

    GPIO_InitStruct.Pin = device->para.cts_pin;
    HAL_GPIO_Init(device->para.cts_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = device->para.rts_pin;
    HAL_GPIO_Init(device->para.rts_port, &GPIO_InitStruct);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_open                                                     */
/* 功能描述: MAX232设备操作之开启                                             */
/* 输入参数: dev --- MAX232设备句柄                                           */
/*           flag --- 打开标志                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max232_open(struct device *dev, int flag)
{
    unsigned char *pbuf = NULL;
    struct max232_device *device = (struct max232_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    pbuf = (unsigned char *)rt_malloc(device->para.fifosz);
#else
    pbuf = (unsigned char *)malloc(device->para.fifosz);
#endif

    assert_param(pbuf);
    memset(pbuf, 0, device->para.fifosz);
    mfifo_init(&device->para.fifo, pbuf, device->para.fifosz);

#ifdef  BSP_USE_RT_THREAD
    rt_thread_startup(device->task);
#endif

    /* MAX232外设启动(FIFO, rx_indicate, tx_indicate) */
    device->uartx->uart_cfg->pfifo = &device->para.fifo;
    device->uartx->uart_dev.rxind  =  device->dev.rxind;
    device->uartx->uart_dev.txind  =  device->dev.txind;
    
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_close                                                    */
/* 功能描述: MAX232设备操作之关闭                                             */
/* 输入参数: dev --- MAX232设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max232_close(struct device *dev)
{
    struct max232_device *device = (struct max232_device *)dev;
    assert_param(device);

    Fn_device_unregister(&device->dev);
    /* MAX232外设关闭 */
    device->uartx->uart_cfg->pfifo = NULL;
    device->uartx->uart_dev.rxind  = NULL;
    device->uartx->uart_dev.txind  = NULL;

#ifdef  MAX232_USE_FLOW_CTRL
    HAL_GPIO_DeInit(device->para.cts_port, device->para.cts_pin);
    HAL_GPIO_DeInit(device->para.rts_port, device->para.rts_pin);
#endif

#ifdef  BSP_USE_RT_THREAD
    rt_thread_delete(device->task);
    rt_mutex_delete(device->lock);
    rt_sem_delete(device->rx_sem);
    rt_sem_delete(device->tx_sem);

    rt_free(device->para.fifo.buffer);
    rt_free(device);
#else
    free(device->para.fifo.buffer);
    free(device);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_read                                                     */
/* 功能描述: MAX232设备操作之读取size长数据到pbuf内                          */
/* 输入参数: dev --- MAX232设备句柄                                           */
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
static int _max232_read(struct device *dev, int pos, void *pbuf, int size)
{   
    int    rlen = 0;
    struct max232_device *device = (struct max232_device *)dev;
    assert_param(device);
    
    if (size <= 0)  return size;
    
#ifdef  BSP_USE_RT_THREAD
    Max232_Lock(device);
#endif

    rlen = Fn_device_read(&device->uartx->uart_dev, pos, pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    Max232_Unlock(device);
#endif

    return (rlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_write                                                    */
/* 功能描述: MAX232设备操作之发送pbuf指向的size长数据                        */
/* 输入参数: dev --- MAX232设备句柄                                           */
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
static int _max232_write(struct device *dev, int pos, const void *pbuf, int size)
{
    int    wlen = 0;
    struct max232_device *device = (struct max232_device *)dev;
    assert_param(device);

    if (size <= 0)  return size;
    
#ifdef  BSP_USE_RT_THREAD
    Max232_Lock(device);
#endif

    wlen = Fn_device_write(&device->uartx->uart_dev, pos, pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    Max232_Unlock(device);
#endif

    return (wlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_control                                                  */
/* 功能描述: MAX232设备操作函数之控制                                        */
/* 输入参数: dev --- MAX232设备句柄                                           */
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
static int _max232_control(struct device *dev, int cmd, void *args)
{
    struct max232_device *device = (struct max232_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    Max232_Lock(device);
#endif

    switch (cmd)
    {
    /* nothing */
    default:
        break;
    }

#ifdef  BSP_USE_RT_THREAD
    Max232_Unlock(device);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_rxindicate                                               */
/* 功能描述: MAX232设备操作函数之接收回调                                    */
/* 输入参数: dev --- MAX232设备句柄                                           */
/*           size --- 收到的数据长度                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max232_rxindicate(struct device *dev, int size)
{
    struct max232_device *device = (struct max232_device *)dev->user;
    assert_param(device);
    
#ifdef  BSP_USE_RT_THREAD
    rt_sem_release(device->rx_sem);
#else

#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max232_txcomplete                                               */
/* 功能描述: MAX232设备操作函数之发送完成回调                                */
/* 输入参数: dev --- MAX232设备句柄                                           */
/*           buffer --- 已发送完毕的数据区指针                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max232_txcomplete(struct device *dev, void *buffer)
{
    struct max232_device *device = (struct max232_device *)dev->user;
    assert_param(device);
    
#ifdef  BSP_USE_RT_THREAD
    rt_sem_release(device->tx_sem);
#else

#endif

    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_max232_init                                                  */
/* 功能描述: MAX232设备初始化和注册函数                                      */
/* 输入参数: device --- MAX232设备句柄                                        */
/*           name --- MAX232设备名称                                          */
/*           bus_name --- SPI总线名称                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_max232_init(const char *name, const char *bus_name)
{
    struct device         *dev = NULL;
    struct _uart_bus_dev  *bus_dev = NULL;
    struct max232_device  *device = NULL;

    assert_param(name);
    assert_param(bus_name);
    assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
    assert_param(sizeof(bus_name) <= DEVICE_NAME_MAXLEN);

#ifdef  BSP_USE_RT_THREAD
    device = (struct max232_device *)rt_malloc(sizeof(struct max232_device));
#else
    device = (struct max232_device *)malloc(sizeof(struct max232_device));
#endif

    gMax232 = device;  
    assert_param(gMax232);
    memset(device, 0, sizeof(struct max232_device));

    bus_dev = (struct _uart_bus_dev *)Fn_device_find(bus_name);
    assert_param(bus_dev);
    device->uartx = bus_dev;
    device->uartx->uart_dev.user = device;    /* 内层对象访问外层对象的关键 */

    device->para.fifosz   = MAX232_UART_FIFOSZ;
#ifdef  MAX232_USE_FLOW_CTRL
    device->para.cts_pin  = MAX232_CTS_PIN;
    device->para.cts_port = MAX232_CTS_PORT;
    device->para.rts_pin  = MAX232_RTS_PIN;
    device->para.rts_port = MAX232_RTS_PORT;
#endif

#ifdef  BSP_USE_RT_THREAD
    struct rt_thread *task = NULL;
    struct rt_mutex  *lock = NULL;
    struct rt_semaphore *sem = NULL;
    
    task = rt_thread_create("max232_task", Max232_Parser, device, 256, 1, 10);
    assert_param(task);
    device->task = task;

    lock = rt_mutex_create("max232_lock", RT_IPC_FLAG_FIFO);
    assert_param(lock);
    device->lock = lock;

    sem = rt_sem_create("max232_rxsem", 0, RT_IPC_FLAG_FIFO);
    assert_param(sem);
    device->rx_sem = sem;

    sem = rt_sem_create("max232_txsem", 0, RT_IPC_FLAG_FIFO);
    assert_param(sem);
    device->tx_sem = sem;
#endif

    dev = &device->dev;
    strcpy(dev->name, name);

    dev->init    = _max232_init;
    dev->open    = _max232_open;
    dev->close   = _max232_close;
    dev->read    = _max232_read;
    dev->write   = _max232_write;
    dev->control = _max232_control;
    dev->rxind   = _max232_rxindicate;
    dev->txind   = _max232_txcomplete;
    
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

int _bsp_max232_init(void)
{
    return bsp_max232_init(DEVICE_NAME, DRIVER_NAME); 
}
INIT_DEVICE_EXPORT(_bsp_max232_init);

