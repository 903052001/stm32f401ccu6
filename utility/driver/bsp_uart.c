/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_uart.c                                                       */
/* 内容摘要: 串口驱动初始化源文件                                            */
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
#include "bsp_uart.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/


/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/
/*<ENUM+>**********************************************************************/
/* 枚举: _UART_BUS_DEV_INDEX                                                  */
/* 注释: UART总线设备枚举                                                    */
/*<ENUM->**********************************************************************/
enum _uart_bus_dev_index
{
#ifdef BSP_USE_UART1
    UART1_INDEX,
#endif

#ifdef BSP_USE_UART2
    UART2_INDEX,
#endif

#ifdef BSP_USE_UART3
    UART3_INDEX,
#endif

#ifdef BSP_USE_UART4
    UART4_INDEX,
#endif

#ifdef BSP_USE_UART5
    UART5_INDEX,
#endif

#ifdef BSP_USE_UART6
    UART6_INDEX,
#endif
};

/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int _uart_bus_init(struct device *dev);
static int _uart_bus_open(struct device *dev, int flag);
static int _uart_bus_close(struct device *dev);
static int _uart_bus_read(struct device *dev, int pos, void *rx, int size);
static int _uart_bus_write(struct device *dev, int pos, const void *tx, int size);
static int _uart_bus_control(struct device *dev, int cmd, void *args);
static int _uart_bus_xfer(struct device *dev, void *msgs, int num);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
struct _uart_bus_cfg uart_bus_devs_cfg =          /* 串口默认配置 9600-8-N-1 */
{
    .baud   = 9600,
    .width  = UART_WORDLENGTH_8B,
    .parity = UART_PARITY_NONE,
    .stop   = UART_STOPBITS_1,
    .pfifo  = NULL,
};

#ifdef BSP_USE_UART1
static UART_HandleTypeDef huart1;                               /* 设备实体 */
struct _uart_bus_hal uart_bus_dev1_hal =
{
    .uartx  = USART1,
    .irqx   = USART1_IRQn,
    .huartx = &huart1,
};

static struct _uart_bus_dev uart_bus_dev1 =
{
    .uart_name = "uart1",
    .uart_hal  = &uart_bus_dev1_hal,
    .uart_cfg  = &uart_bus_devs_cfg,
};
#endif

#ifdef BSP_USE_UART2
static UART_HandleTypeDef huart2;
struct _uart_bus_hal uart_bus_dev2_hal =
{
    .uartx  = USART2,
    .irqx   = USART2_IRQn,
    .huartx = &huart2,
};

static struct _uart_bus_dev uart_bus_dev2 =
{
    .uart_name = "uart2",
    .uart_hal  = &uart_bus_dev2_hal,
    .uart_cfg  = &uart_bus_devs_cfg,
};
#endif

#ifdef BSP_USE_UART3
static UART_HandleTypeDef huart3;
struct _uart_bus_hal uart_bus_dev3_hal =
{
    .uartx  = USART3,
    .irqx   = USART3_IRQn,
    .huartx = &huart3,
};

static struct _uart_bus_dev uart_bus_dev3 =
{
    .uart_name = "uart3",
    .uart_hal  = &uart_bus_dev3_hal,
    .uart_cfg  = &uart_bus_devs_cfg,
};
#endif

#ifdef BSP_USE_UART4
static UART_HandleTypeDef huart4;
struct _uart_bus_hal uart_bus_dev4_hal =
{
    .uartx  = UART4,
    .irqx   = UART4_IRQn,
    .huartx = &huart4,
};

static struct _uart_bus_dev uart_bus_dev4 =
{
    .uart_name = "uart4",
    .uart_hal  = &uart_bus_dev4_hal,
    .uart_cfg  = &uart_bus_devs_cfg,
};
#endif

#ifdef BSP_USE_UART5
static UART_HandleTypeDef huart5;
struct _uart_bus_hal uart_bus_dev5_hal =
{
    .uartx  = UART5,
    .irqx   = UART5_IRQn,
    .huartx = &huart5,
};

static struct _uart_bus_dev uart_bus_dev5 =
{
    .uart_name = "uart5",
    .uart_hal  = &uart_bus_dev5_hal,
    .uart_cfg  = &uart_bus_devs_cfg,
};
#endif

#ifdef BSP_USE_UART6
static UART_HandleTypeDef huart6;
struct _uart_bus_hal uart_bus_dev6_hal =
{
    .uartx  = USART6,
    .irqx   = USART6_IRQn,
    .huartx = &huart6,
};

static struct _uart_bus_dev uart_bus_dev6 =
{
    .uart_name = "uart6",
    .uart_hal  = &uart_bus_dev6_hal,
    .uart_cfg  = &uart_bus_devs_cfg,
};
#endif

static struct _uart_bus_dev *uart_bus_devs[] =
{
#ifdef BSP_USE_UART1
    &uart_bus_dev1,
#endif

#ifdef BSP_USE_UART2
    &uart_bus_dev2,
#endif

#ifdef BSP_USE_UART3
    &uart_bus_dev3,
#endif

#ifdef BSP_USE_UART4
    &uart_bus_dev4,
#endif

#ifdef BSP_USE_UART5
    &uart_bus_dev5,
#endif

#ifdef BSP_USE_UART6
    &uart_bus_dev6,
#endif
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/******************************************************************************/
/* printf  输出重定向,加入以下代码,支持printf函数,不需要选择use MicroLIB     */
/******************************************************************************/
#if 0
#define CONSOLE              USART1                 /* 控制台串口 */
#pragma import(__use_no_semihosting)

struct __FILE     /* 标准库需要的支持函数 */
{
    int handle;
};

FILE __stdout;

void _sys_exit(int x)    /* 定义_sys_exit()以避免使用半主机模式 */
{
    x = x;
}

int fputc(int ch, FILE *f)   /* 重定义fputc函数 */
{
    CONSOLE->SR;
    CONSOLE->DR = (unsigned char)ch;
    while (!(CONSOLE->SR & 0x40));
    return ch;
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_init                                                   */
/* 功能描述: UART总线操作之初始化                                             */
/* 输入参数: dev --- UART总线设备句柄                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_init(struct device *dev)
{
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);

    uart_bus->uart_hal->huartx->Instance = uart_bus->uart_hal->uartx;
    uart_bus->uart_hal->huartx->Init.BaudRate = uart_bus->uart_cfg->baud;
    uart_bus->uart_hal->huartx->Init.WordLength = uart_bus->uart_cfg->width;
    uart_bus->uart_hal->huartx->Init.StopBits = uart_bus->uart_cfg->stop;
    uart_bus->uart_hal->huartx->Init.Parity = uart_bus->uart_cfg->parity;
    uart_bus->uart_hal->huartx->Init.Mode = UART_MODE_TX_RX;
    uart_bus->uart_hal->huartx->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_bus->uart_hal->huartx->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(uart_bus->uart_hal->huartx) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_open                                                   */
/* 功能描述: UART总线操作之开启                                               */
/* 输入参数: dev --- UART总线设备句柄                                         */
/*           flag --- 打开参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_open(struct device *dev, int flag)
{
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);

    HAL_NVIC_EnableIRQ(uart_bus->uart_hal->irqx);
    __HAL_UART_ENABLE(uart_bus->uart_hal->huartx);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_close                                                  */
/* 功能描述: UART总线操作之关闭                                               */
/* 输入参数: dev --- UART总线设备句柄                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_close(struct device *dev)
{
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);

    HAL_NVIC_DisableIRQ(uart_bus->uart_hal->irqx);
    __HAL_UART_DISABLE(uart_bus->uart_hal->huartx);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_read                                                   */
/* 功能描述: UART总线操作之读取数据                                           */
/* 输入参数: dev --- UART总线设备句柄                                         */
/*           pos --- 从设备哪里读                                             */
/*           rx --- 接收缓存区                                                */
/*           size --- 读取长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 使用rt_thread后,HAL库函数的超时将不起作用                       */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_read(struct device *dev, int pos, void *rx, int size)
{
    int    rlen = 0;
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);
    assert_param(size > 0);

    if (NULL != uart_bus->uart_cfg->pfifo)
    {
        rlen = mfifo_get(uart_bus->uart_cfg->pfifo, (unsigned char *)rx, size);
    }

    return rlen;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_write                                                  */
/* 功能描述: UART总线操作之发送数据                                           */
/* 输入参数: dev --- UART总线设备句柄                                         */
/*           pos --- 写入到设备哪里                                           */
/*           tx --- 发送缓存区                                                */
/*           size --- 发送长度                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_write(struct device *dev, int pos, const void *tx, int size)
{
      signed int  wlen = 0;
    unsigned char *ptx = (unsigned char *)tx;
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);
    assert_param(size > 0);

    while (size--)
    {
        while (__HAL_UART_GET_FLAG(uart_bus->uart_hal->huartx, UART_FLAG_TC) == RESET);

        if (HAL_UART_Transmit(uart_bus->uart_hal->huartx, ptx++, 1, 5) == HAL_OK)
        {
            wlen++;
        }
    }

    return wlen;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_control                                                */
/* 功能描述: UART总线操作之控制                                               */
/* 输入参数: dev --- UART总线设备句柄                                         */
/*           cmd --- 控制命令                                                 */
/*           args --- 控制参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_control(struct device *dev, int cmd, void *args)
{
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);

    switch(cmd)
    {
    /* nothing */
    
    default:
      break;
    }
    
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _uart_bus_xfer                                                   */
/* 功能描述: UART总线读写统一操作函数                                        */
/* 输入参数: dev --- UART总线设备句柄                                         */
/*           msgs --- UART数据结构体指针                                      */
/*           num --- UART数据结构体个数                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _uart_bus_xfer(struct device *dev, void *msgs, int num)
{
    struct _uart_bus_dev *uart_bus = (struct _uart_bus_dev *)dev;
    assert_param(uart_bus);

    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_uart_init                                                    */
/* 功能描述: UART总线初始化和注册函数                                        */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_uart_init(void)
{
    int    i;
    const  char  *name = NULL;
    struct device *dev = NULL;

    for (i = 0; i < sizeof(uart_bus_devs) / sizeof(struct _uart_bus_dev *); i++)
    {
        name = uart_bus_devs[i]->uart_name;
        dev = &uart_bus_devs[i]->uart_dev;

        assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
        strcpy(dev->name, name);

        dev->init    = _uart_bus_init;
        dev->open    = _uart_bus_open;
        dev->close   = _uart_bus_close;
        dev->read    = _uart_bus_read;
        dev->write   = _uart_bus_write;
        dev->control = _uart_bus_control;
        dev->xfer    = _uart_bus_xfer;

        if (0 != Fn_device_register(dev, name))
        {
            Error_Handler(__FILE__, __LINE__);
        }

        if (0 != Fn_device_open(dev, 0))
        {
            Error_Handler(__FILE__, __LINE__);
        }
    }

    return 0;
}

int _bsp_uart_init(void)
{
    return bsp_uart_init();
}
INIT_BOARD_EXPORT(_bsp_uart_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_uart_isr                                                     */
/* 功能描述: 串口中断统一处理函数                                             */
/* 输入参数: index --- 串口索引                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void bsp_uart_isr(enum _uart_bus_dev_index index)
{
    int rlen;
    unsigned char c;

#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_enter(); 
#else
    __disable_irq();
#endif

    if (__HAL_UART_GET_FLAG(uart_bus_devs[index]->uart_hal->huartx, UART_FLAG_RXNE) != RESET)
    {   
        c = uart_bus_devs[index]->uart_hal->huartx->Instance->DR & 0xFF;
    
        if (NULL != uart_bus_devs[index]->uart_cfg->pfifo)
        {
            mfifo_put(uart_bus_devs[index]->uart_cfg->pfifo, &c, 1);
        }
    }
    else if (__HAL_UART_GET_FLAG(uart_bus_devs[index]->uart_hal->huartx, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(uart_bus_devs[index]->uart_hal->huartx);
    
        if (NULL != uart_bus_devs[index]->uart_dev.rxind)
        {
            rlen = RING_D_VAL(uart_bus_devs[index]->uart_cfg->pfifo->in, 
                              uart_bus_devs[index]->uart_cfg->pfifo->out, 
                              uart_bus_devs[index]->uart_cfg->pfifo->size);
              
            if (rlen > 0)
            {
                uart_bus_devs[index]->uart_dev.rxind(&uart_bus_devs[index]->uart_dev, rlen);
            }
        }
    }
    else
    {
        if (__HAL_UART_GET_FLAG(uart_bus_devs[index]->uart_hal->huartx, UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(uart_bus_devs[index]->uart_hal->huartx);
        }
        
        if (__HAL_UART_GET_FLAG(uart_bus_devs[index]->uart_hal->huartx, UART_FLAG_NE) != RESET)
        {
            __HAL_UART_CLEAR_NEFLAG(uart_bus_devs[index]->uart_hal->huartx);
        }
        
        if (__HAL_UART_GET_FLAG(uart_bus_devs[index]->uart_hal->huartx, UART_FLAG_FE) != RESET)
        {
            __HAL_UART_CLEAR_FEFLAG(uart_bus_devs[index]->uart_hal->huartx);
        }
        
        if (__HAL_UART_GET_FLAG(uart_bus_devs[index]->uart_hal->huartx, UART_FLAG_PE) != RESET)
        {
            __HAL_UART_CLEAR_PEFLAG(uart_bus_devs[index]->uart_hal->huartx);
        }
    }

#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_leave(); 
#else
    __enable_irq();
#endif

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: USARTx_IRQHandler                                                */
/* 功能描述: 串口中断入口函数                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
#ifdef  BSP_USE_UART1
void USART1_IRQHandler(void)
{
    bsp_uart_isr(UART1_INDEX);
}
#endif

#ifdef  BSP_USE_UART2
void USART2_IRQHandler(void)
{
    bsp_uart_isr(UART2_INDEX);
}
#endif

#ifdef  BSP_USE_UART3
void USART3_IRQHandler(void)
{
    bsp_uart_isr(UART3_INDEX);
}
#endif

#ifdef  BSP_USE_UART4
void UART4_IRQHandler(void)
{
    bsp_uart_isr(UART4_INDEX);
}
#endif

#ifdef  BSP_USE_UART5
void UART5_IRQHandler(void)
{
    bsp_uart_isr(UART5_INDEX);
}
#endif

#ifdef  BSP_USE_UART6
void USART6_IRQHandler(void)
{
    bsp_uart_isr(UART6_INDEX);
}
#endif

