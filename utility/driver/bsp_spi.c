/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_spi.c                                                        */
/* 内容摘要: SPI驱动初始化源文件                                             */
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
#include "bsp_spi.h"

/******************************************************************************/
/*                                外部引用声明                               */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                */
/******************************************************************************/


/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/
/*<ENUM+>**********************************************************************/
/* 枚举: _SPI_BUS_DEV_INDEX                                                   */
/* 注释: SPI总线设备枚举                                                      */
/*<ENUM->**********************************************************************/
enum _spi_bus_dev_index
{
#ifdef BSP_USE_SPI1
    SPI1_INDEX,
#endif

#ifdef BSP_USE_SPI2
    SPI2_INDEX,
#endif

#ifdef BSP_USE_SPI3
    SPI3_INDEX,
#endif
};

/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int _spi_bus_init(struct device *dev);
static int _spi_bus_open(struct device *dev, int flag);
static int _spi_bus_close(struct device *dev);
static int _spi_bus_xfer(struct device *dev, void *msgs, int num);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
#ifdef BSP_USE_SPI1
static SPI_HandleTypeDef hspi1;                                /* 设备实体 */
struct _spi_bus_hal spi_bus_dev1_hal =
{
    .spix  = SPI1,
    .irqx  = SPI1_IRQn,
    .hspix = &hspi1,
};

struct _spi_bus_cfg spi_bus_dev1_cfg =
{
    .polarity = SPI_POLARITY_HIGH,
    .phase    = SPI_PHASE_2EDGE,
    .baud     = SPI_BAUDRATEPRESCALER_256,
    .width    = SPI_DATASIZE_8BIT,
    .firstbit = SPI_FIRSTBIT_MSB,
};

static struct _spi_bus_dev spi_bus_dev1 =
{
    .spi_name = "spi1",
    .spi_hal  = &spi_bus_dev1_hal,
    .spi_cfg  = &spi_bus_dev1_cfg,
};
#endif

#ifdef BSP_USE_SPI2
static SPI_HandleTypeDef hspi2;
struct _spi_bus_hal spi_bus_dev2_hal =
{
    .spix  = SPI2,
    .irqx  = SPI2_IRQn,
    .hspix = &hspi2,
};

struct _spi_bus_cfg spi_bus_dev2_cfg =
{
    .polarity = SPI_POLARITY_HIGH,
    .phase    = SPI_PHASE_2EDGE,
    .baud     = SPI_BAUDRATEPRESCALER_256,
    .width    = SPI_DATASIZE_8BIT,
    .firstbit = SPI_FIRSTBIT_MSB,
};

static struct _spi_bus_dev spi_bus_dev2 =
{
    .spi_name = "spi2",
    .spi_hal  = &spi_bus_dev2_hal,
    .spi_cfg  = &spi_bus_dev2_cfg,
};
#endif

#ifdef BSP_USE_SPI3
static SPI_HandleTypeDef hspi3;
struct _spi_bus_hal spi_bus_dev3_hal =
{
    .spix  = SPI3,
    .irqx  = SPI3_IRQn,
    .hspix = &hspi3,
};

struct _spi_bus_cfg spi_bus_dev3_cfg =
{
    .polarity = SPI_POLARITY_HIGH,
    .phase    = SPI_PHASE_2EDGE,
    .baud     = SPI_BAUDRATEPRESCALER_256,
    .width    = SPI_DATASIZE_8BIT,
    .firstbit = SPI_FIRSTBIT_MSB,
};

static struct _spi_bus_dev spi_bus_dev3 =
{
    .spi_name = "spi3",
    .spi_hal  = &spi_bus_dev3_hal,
    .spi_cfg  = &spi_bus_dev3_cfg,
};
#endif

static struct _spi_bus_dev *spi_bus_devs[] =
{
#ifdef BSP_USE_SPI1
    &spi_bus_dev1,
#endif

#ifdef BSP_USE_SPI2
    &spi_bus_dev2,
#endif

#ifdef BSP_USE_SPI3
    &spi_bus_dev3,
#endif
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: _spi_bus_init                                                    */
/* 功能描述: SPI总线操作之初始化                                             */
/* 输入参数: dev --- SPI总线设备句柄                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _spi_bus_init(struct device *dev)
{
    struct _spi_bus_dev *spi_bus = (struct _spi_bus_dev *)dev;
    assert_param(spi_bus);

    spi_bus->spi_hal->hspix->Instance = spi_bus->spi_hal->spix;
    spi_bus->spi_hal->hspix->Init.Mode = SPI_MODE_MASTER;
    spi_bus->spi_hal->hspix->Init.Direction = SPI_DIRECTION_2LINES;
    spi_bus->spi_hal->hspix->Init.DataSize = spi_bus->spi_cfg->width;
    spi_bus->spi_hal->hspix->Init.CLKPolarity = spi_bus->spi_cfg->polarity;
    spi_bus->spi_hal->hspix->Init.CLKPhase = spi_bus->spi_cfg->phase;
    spi_bus->spi_hal->hspix->Init.NSS = SPI_NSS_SOFT;
    spi_bus->spi_hal->hspix->Init.BaudRatePrescaler = spi_bus->spi_cfg->baud;
    spi_bus->spi_hal->hspix->Init.FirstBit = spi_bus->spi_cfg->firstbit;
    spi_bus->spi_hal->hspix->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_bus->spi_hal->hspix->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_bus->spi_hal->hspix->Init.CRCPolynomial = 7;

    if (HAL_SPI_Init(spi_bus->spi_hal->hspix) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _spi_bus_open                                                    */
/* 功能描述: SPI总线操作之开启                                                */
/* 输入参数: dev --- SPI总线设备句柄                                          */
/*           flag --- 打开参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _spi_bus_open(struct device *dev, int flag)
{
    struct _spi_bus_dev *spi_bus = (struct _spi_bus_dev *)dev;
    assert_param(spi_bus);

    HAL_NVIC_EnableIRQ(spi_bus->spi_hal->irqx);
    __HAL_SPI_ENABLE(spi_bus->spi_hal->hspix);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _spi_bus_close                                                   */
/* 功能描述: SPI总线操作之关闭                                                */
/* 输入参数: dev --- SPI总线设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _spi_bus_close(struct device *dev)
{
    struct _spi_bus_dev *spi_bus = (struct _spi_bus_dev *)dev;
    assert_param(spi_bus);

    HAL_NVIC_DisableIRQ(spi_bus->spi_hal->irqx);
    __HAL_SPI_DISABLE(spi_bus->spi_hal->hspix);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _spi_bus_xfer                                                    */
/* 功能描述: SPI总线读写统一操作函数                                          */
/* 输入参数: dev --- SPI总线设备句柄                                          */
/*           msgs --- SPI数据结构体指针                                       */
/*           num --- SPI数据结构体个数                                        */
/* 输出参数: 返回收到的1字节数据                                             */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _spi_bus_xfer(struct device *dev, void *msgs, int num)
{
    unsigned char  rx;
    unsigned char *tx = (unsigned char *)msgs;
    struct _spi_bus_dev *spi_bus = (struct _spi_bus_dev *)dev;

    assert_param(tx);
    assert_param(spi_bus);
    assert_param(1 == num);

    HAL_SPI_TransmitReceive(spi_bus->spi_hal->hspix, tx, &rx, num, 5);

    return (rx & 0xFF);
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_spi_init                                                     */
/* 功能描述: SPI总线初始化和注册函数                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_spi_init(void)
{
    int    i;
    const  char  *name = NULL;
    struct device *dev = NULL;

    for (i = 0; i < sizeof(spi_bus_devs) / sizeof(struct _spi_bus_dev *); i++)
    {
        name = spi_bus_devs[i]->spi_name;
        dev = &spi_bus_devs[i]->spi_dev;

        assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
        strcpy(dev->name, name);

        dev->init  = _spi_bus_init;
        dev->open  = _spi_bus_open;
        dev->close = _spi_bus_close;
        dev->xfer  = _spi_bus_xfer;

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

int _bsp_spi_init(void)
{
    return bsp_spi_init();
}
INIT_BOARD_EXPORT(_bsp_spi_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_spi_isr                                                      */
/* 功能描述: SPI中断统一处理函数                                             */
/* 输入参数: index --- SPI总线索引                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void bsp_spi_isr(enum _spi_bus_dev_index index)
{
#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_enter(); 
#else
    __disable_irq();
#endif

    if (__HAL_SPI_GET_FLAG(spi_bus_devs[index]->spi_hal->hspix, SPI_FLAG_RXNE))
    {

    }
    else if (__HAL_SPI_GET_FLAG(spi_bus_devs[index]->spi_hal->hspix, SPI_FLAG_TXE))
    {

    }

#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_leave(); 
#else
    __enable_irq();
#endif

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: SPIx_IRQHandler                                                  */
/* 功能描述: SPI中断入口函数                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
#ifdef  BSP_USE_SPI1
void SPI1_IRQHandler(void)
{
    bsp_spi_isr(SPI1_INDEX);
}
#endif

#ifdef  BSP_USE_SPI2
void SPI2_IRQHandler(void)
{
    bsp_spi_isr(SPI2_INDEX);
}
#endif

#ifdef  BSP_USE_SPI3
void SPI3_IRQHandler(void)
{
    bsp_spi_isr(SPI3_INDEX);
}
#endif

