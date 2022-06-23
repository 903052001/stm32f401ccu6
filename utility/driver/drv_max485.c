/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: drv_max485.c                                                     */
/* 内容摘要: MAX485芯片驱动源文件                                            */
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
#include "drv_max485.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  MAX485_UART_FIFOSZ    256                   /* MAX485外设fifo总大小 */
#define  MAX485_UART_BAUD      115200                /* MAX485外设串口波特率 */
#define  MAX485_UART_WIDTH     UART_WORDLENGTH_8B    /* MAX485外设数据位宽   */
#define  MAX485_UART_PARITY    UART_PARITY_NONE      /* MAX485外设校验方式   */
#define  MAX485_UART_STOP      UART_STOPBITS_1       /* MAX485外设停止位     */

#ifdef   MAX485_USE_FLOW_CTRL
#define  MAX485_CTS_PORT       GPIOA                 /* 默认USART1流控脚     */
#define  MAX485_CTS_PIN        GPIO_PIN_11
#define  MAX485_RTS_PORT       GPIOA
#define  MAX485_RTS_PIN        GPIO_PIN_12
#endif

#define  MAX485_DIR_PORT       GPIOA
#define  MAX485_DIR_PIN        GPIO_PIN_8

#define  DEVICE_NAME           "MAX485"
#define  DRIVER_NAME           "uart6"

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/
/*<ENUM+>**********************************************************************/
/* 枚举: _MAX485_DIR                                                          */
/* 注释: r/w方向枚举                                                          */
/*<ENUM->**********************************************************************/
enum _max485_dir
{
    RECV = 0,
    SEND = !RECV,
};

/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int  Max485_Lock(struct max485_device *dev);
static int  Max485_Unlock(struct max485_device *dev);
#endif

static void BSP_MAX485_DIR(struct max485_device *dev, enum _max485_dir dir);
static void Max485_Parser(void *parameter);
static int _max485_init(struct device *dev);
static int _max485_open(struct device *dev, int flga);
static int _max485_close(struct device *dev);
static int _max485_read(struct device *dev, int pos, void *pbuf, int size);
static int _max485_write(struct device *dev, int pos, const void *pbuf, int size);
static int _max485_control(struct device *dev, int cmd, void *args);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static struct max485_device *gMax485 = NULL;                     /* 设备指针 */

static struct _uart_bus_cfg _max485_uart_cfg =        /* MAX485外设串口参数  */
{
    .baud   = MAX485_UART_BAUD,
    .width  = MAX485_UART_WIDTH,
    .parity = MAX485_UART_PARITY,
    .stop   = MAX485_UART_STOP,
    .pfifo  = NULL,
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef  BSP_USE_RT_THREAD
static int Max485_Lock(struct max485_device *dev)
{
    return rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static int Max485_Unlock(struct max485_device *dev)
{
    return rt_mutex_release(dev->lock);
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: Max485_Parser                                                    */
/* 功能描述: MAX485数据协议解析器                                             */
/* 输入参数: parameter --- 参数指针                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void Max485_Parser(void *parameter)
{
    unsigned char ch = 0;
    struct max485_device *device = (struct max485_device *)parameter;

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
/* 函数名称: BSP_MAX485_DIR                                                   */
/* 功能描述: DIR引脚的控制                                                    */
/* 输入参数: dev --- MAX485设备句柄                                           */
/*           dir --- DIR引脚状态；0-接收模式，非0-输出模式                   */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void BSP_MAX485_DIR(struct max485_device *dev, enum _max485_dir dir)
{
    if (RECV == dir)
    {
        HAL_GPIO_WritePin(dev->para.dir_port, dev->para.dir_pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(dev->para.dir_port, dev->para.dir_pin, GPIO_PIN_SET);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_init                                                     */
/* 功能描述: MAX485设备操作之初始化                                           */
/* 输入参数: dev --- MAX485设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 重设串口外设相关配置参数以及创建fifo                            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max485_init(struct device *dev)
{
    struct max485_device *device = (struct max485_device *)dev;
    assert_param(device);

    /* 重新配置串口 */
    device->uartx->uart_cfg = &_max485_uart_cfg;
    Fn_device_init(&device->uartx->uart_dev);

    GPIO_InitTypeDef GPIO_InitStruct;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = device->para.dir_pin;
    HAL_GPIO_Init(device->para.dir_port, &GPIO_InitStruct);
    
    BSP_MAX485_DIR(device, RECV);    /* 默认接收模式 */
    
#ifdef  MAX485_USE_FLOW_CTRL
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;    /* 流控必须此模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;

    GPIO_InitStruct.Pin = device->para.cts_pin;
    HAL_GPIO_Init(device->para.cts_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = device->para.rts_pin;
    HAL_GPIO_Init(device->para.rts_port, &GPIO_InitStruct);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_open                                                     */
/* 功能描述: MAX485设备操作之开启                                             */
/* 输入参数: dev --- MAX485设备句柄                                           */
/*           flag --- 打开标志                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max485_open(struct device *dev, int flag)
{
    unsigned char *pbuf = NULL;
    struct max485_device *device = (struct max485_device *)dev;
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

    /* MAX485外设启动(FIFO, rx_indicate, tx_indicate) */
    device->uartx->uart_cfg->pfifo = &device->para.fifo;
    device->uartx->uart_dev.rxind  =  device->dev.rxind;
    device->uartx->uart_dev.txind  =  device->dev.txind;

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_close                                                    */
/* 功能描述: MAX485设备操作之关闭                                             */
/* 输入参数: dev --- MAX485设备句柄                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max485_close(struct device *dev)
{
    struct max485_device *device = (struct max485_device *)dev;
    assert_param(device);

    Fn_device_unregister(&device->dev);
    /* MAX485外设FIFO关闭 */
    device->uartx->uart_cfg->pfifo = NULL;
    device->uartx->uart_dev.rxind  = NULL;
    device->uartx->uart_dev.txind  = NULL;
    
    HAL_GPIO_DeInit(device->para.dir_port, device->para.dir_pin);

#ifdef  MAX485_USE_FLOW_CTRL
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
/* 函数名称: _max485_read                                                     */
/* 功能描述: MAX485设备操作之读取size长数据到pbuf内                          */
/* 输入参数: dev --- MAX485设备句柄                                           */
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
static int _max485_read(struct device *dev, int pos, void *pbuf, int size)
{
    int    rlen = 0;
    struct max485_device *device = (struct max485_device *)dev;
    assert_param(device);

    if (size <= 0)  return size;

#ifdef  BSP_USE_RT_THREAD
    Max485_Lock(device);
#endif

    rlen = Fn_device_read(&device->uartx->uart_dev, pos, pbuf, size);

#ifdef  BSP_USE_RT_THREAD
    Max485_Unlock(device);
#endif

    return (rlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_write                                                    */
/* 功能描述: MAX485设备操作之发送pbuf指向的size长数据                        */
/* 输入参数: dev --- MAX485设备句柄                                           */
/*           pos --- 写入到设备哪里                                           */
/*           pbuf --- 发送数据区                                              */
/*           size --- 发送长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 如果485读写存在问题,可在读写间加点延时                          */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max485_write(struct device *dev, int pos, const void *pbuf, int size)
{
    int    wlen = 0;
    struct max485_device *device = (struct max485_device *)dev;
    assert_param(device);

    if (size <= 0)  return size;

#ifdef  BSP_USE_RT_THREAD
    Max485_Lock(device);
#endif

    BSP_MAX485_DIR(device, SEND);
    wlen = Fn_device_write(&device->uartx->uart_dev, pos, pbuf, size);
    BSP_MAX485_DIR(device, RECV);

#ifdef  BSP_USE_RT_THREAD
    Max485_Unlock(device);
#endif

    return (wlen == size) ? 0 : -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_control                                                  */
/* 功能描述: MAX485设备操作函数之控制                                        */
/* 输入参数: dev --- MAX485设备句柄                                           */
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
static int _max485_control(struct device *dev, int cmd, void *args)
{
    struct max485_device *device = (struct max485_device *)dev;
    assert_param(device);

#ifdef  BSP_USE_RT_THREAD
    Max485_Lock(device);
#endif

    switch (cmd)
    {
    /* nothing */
    default:
        break;
    }

#ifdef  BSP_USE_RT_THREAD
    Max485_Unlock(device);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_rxindicate                                               */
/* 功能描述: MAX485设备操作函数之接收回调                                    */
/* 输入参数: dev --- MAX485设备句柄                                           */
/*           size --- 收到的数据长度                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max485_rxindicate(struct device *dev, int size)
{
    struct max485_device *device = (struct max485_device *)dev->user;
    assert_param(device);
    
#ifdef  BSP_USE_RT_THREAD
    rt_sem_release(device->rx_sem);
#else

#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _max485_txcomplete                                               */
/* 功能描述: MAX485设备操作函数之发送完成回调                                */
/* 输入参数: dev --- MAX485设备句柄                                           */
/*           buffer --- 已发送完毕的数据区指针                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _max485_txcomplete(struct device *dev, void *buffer)
{
    struct max485_device *device = (struct max485_device *)dev->user;
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
/* 函数名称: bsp_max485_init                                                  */
/* 功能描述: MAX485设备初始化和注册函数                                      */
/* 输入参数: device --- MAX485设备句柄                                        */
/*           name --- MAX485设备名称                                          */
/*           bus_name --- SPI总线名称                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_max485_init(const char *name, const char *bus_name)
{
    struct device         *dev = NULL;
    struct _uart_bus_dev  *bus_dev = NULL;
    struct max485_device  *device = NULL;

    assert_param(name);
    assert_param(bus_name);
    assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
    assert_param(sizeof(bus_name) <= DEVICE_NAME_MAXLEN);

#ifdef  BSP_USE_RT_THREAD
    device = (struct max485_device *)rt_malloc(sizeof(struct max485_device));
#else
    device = (struct max485_device *)malloc(sizeof(struct max485_device));
#endif

    gMax485 = device;  
    assert_param(gMax485);
    memset(device, 0, sizeof(struct max485_device));

    bus_dev = (struct _uart_bus_dev *)Fn_device_find(bus_name);
    assert_param(bus_dev);
    device->uartx = bus_dev;
    device->uartx->uart_dev.user = device;
    
    device->para.dir_port = MAX485_DIR_PORT;
    device->para.dir_pin  = MAX485_DIR_PIN;
    device->para.fifosz   = MAX485_UART_FIFOSZ;
    
#ifdef  MAX485_USE_FLOW_CTRL
    device->para.cts_pin  = MAX485_CTS_PIN;
    device->para.cts_port = MAX485_CTS_PORT;
    device->para.rts_pin  = MAX485_RTS_PIN;
    device->para.rts_port = MAX485_RTS_PORT;
#endif

#ifdef  BSP_USE_RT_THREAD
    struct rt_thread *task = NULL;
    struct rt_mutex  *lock = NULL;
    struct rt_semaphore *sem = NULL;

    task = rt_thread_create("max485_task", Max485_Parser, device, 256, 1, 10);
    assert_param(task);
    device->task = task;

    lock = rt_mutex_create("max485_lock", RT_IPC_FLAG_FIFO);
    assert_param(lock);
    device->lock = lock;

    sem = rt_sem_create("max485_rxsem", 0, RT_IPC_FLAG_FIFO);
    assert_param(sem);
    device->rx_sem = sem;

    sem = rt_sem_create("max485_txsem", 0, RT_IPC_FLAG_FIFO);
    assert_param(sem);
    device->tx_sem = sem;
#endif

    dev = &device->dev;
    strcpy(dev->name, name);

    dev->init    = _max485_init;
    dev->open    = _max485_open;
    dev->close   = _max485_close;
    dev->read    = _max485_read;
    dev->write   = _max485_write;
    dev->control = _max485_control;
    dev->rxind   = _max485_rxindicate;
    dev->txind   = _max485_txcomplete;

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

int _bsp_max485_init(void)
{
    return bsp_max485_init(DEVICE_NAME, DRIVER_NAME); 
}
INIT_DEVICE_EXPORT(_bsp_max485_init);

