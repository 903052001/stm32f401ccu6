/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_time.c                                                       */
/* 内容摘要: TIM驱动初始化源文件(时基定时器)                                 */
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
#include "bsp_time.h"

/******************************************************************************/
/*                                外部引用声明                               */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                */
/******************************************************************************/
#define APB2_CLK                (84000000)            /* APB2定时器时钟频率  */
#define APB1_CLK                (APB2_CLK / 2)        /* APB1定时器时钟频率  */

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/
/*<ENUM+>**********************************************************************/
/* 枚举: _TIM_DEV_INDEX                                                       */
/* 注释: TIM定时器设备枚举                                                   */
/*<ENUM->**********************************************************************/
enum _tim_dev_index
{
#ifdef BSP_USE_TIM1
    TIM1_INDEX,
#endif

#ifdef BSP_USE_TIM2
    TIM2_INDEX,
#endif

#ifdef BSP_USE_TIM3
    TIM3_INDEX,
#endif
};

/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int _tim_dev_init(struct device *dev);
static int _tim_dev_open(struct device *dev, int flag);
static int _tim_dev_close(struct device *dev);
static int _tim_dev_control(struct device *dev, int cmd, void *args);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
#ifdef BSP_USE_TIM1
static TIM_HandleTypeDef htim1;                                /* 设备实体 */
struct _tim_dev_hal tim_dev1_hal =
{
    .timx  = TIM1,
    .irqx  = TIM1_CC_IRQn,
    .htimx = &htim1,
};

struct _tim_dev_cfg tim_dev1_cfg =
{
    .prescaler  = APB2_CLK / 1000 - 1,
    .countmode  = TIM_COUNTERMODE_UP,
    .period     = 99,                                    /* 100ms的时基定时器 */
    .clockdiv   = TIM_CLOCKDIVISION_DIV1,
    .repetcount = 0,
    .autoreload = TIM_AUTORELOAD_PRELOAD_ENABLE,
};

static struct _tim_dev tim_dev1 =
{
    .tim_name = "tim1",
    .tim_hal  = &tim_dev1_hal,
    .tim_cfg  = &tim_dev1_cfg,
};
#endif

#ifdef BSP_USE_TIM2
static TIM_HandleTypeDef htim2;
struct _tim_dev_hal tim_dev2_hal =
{
    .timx  = TIM2,
    .irqx  = TIM2_IRQn,
    .htimx = &htim2,
};

struct _tim_dev_cfg tim_dev2_cfg =
{
    .prescaler  = APB1_CLK / 100 - 1,
    .countmode  = TIM_COUNTERMODE_UP,
    .period     = 99,                                    /* 10ms的时基定时器 */
    .clockdiv   = TIM_CLOCKDIVISION_DIV1,
    .repetcount = 0,
    .autoreload = TIM_AUTORELOAD_PRELOAD_ENABLE,
};

static struct _tim_dev tim_dev2 =
{
    .tim_name = "tim2",
    .tim_hal  = &tim_dev2_hal,
    .tim_cfg  = &tim_dev2_cfg,
};
#endif

#ifdef BSP_USE_TIM3
static TIM_HandleTypeDef htim3;
struct _tim_dev_hal tim_dev3_hal =
{
    .timx  = TIM3,
    .irqx  = TIM3_IRQn,
    .htimx = &htim3,
};

struct _tim_dev_cfg tim_dev3_cfg =
{
    .prescaler  = APB1_CLK / 10 - 1,
    .countmode  = TIM_COUNTERMODE_UP,
    .period     = 9,                                      /* 1ms的时基定时器 */
    .clockdiv   = TIM_CLOCKDIVISION_DIV1,
    .repetcount = 0,
    .autoreload = TIM_AUTORELOAD_PRELOAD_ENABLE,
};

static struct _tim_dev tim_dev3 =
{
    .tim_name = "tim3",
    .tim_hal  = &tim_dev3_hal,
    .tim_cfg  = &tim_dev3_cfg,
};
#endif

static struct _tim_dev *tim_devs[] =
{
#ifdef BSP_USE_TIM1
    &tim_dev1,
#endif

#ifdef BSP_USE_TIM2
    &tim_dev2,
#endif

#ifdef BSP_USE_TIM3
    &tim_dev3,
#endif
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: _tim_dev_init                                                    */
/* 功能描述: TIM定时器操作之初始化                                           */
/* 输入参数: dev --- TIM定时器设备句柄                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _tim_dev_init(struct device *dev)
{
    struct _tim_dev *tim_dev = (struct _tim_dev *)dev;
    assert_param(tim_dev);

    tim_dev->tim_hal->htimx->Instance = tim_dev->tim_hal->timx;
    tim_dev->tim_hal->htimx->Init.Prescaler = tim_dev->tim_cfg->prescaler;
    tim_dev->tim_hal->htimx->Init.CounterMode = tim_dev->tim_cfg->countmode;
    tim_dev->tim_hal->htimx->Init.Period = tim_dev->tim_cfg->period;
    tim_dev->tim_hal->htimx->Init.ClockDivision = tim_dev->tim_cfg->clockdiv;
    tim_dev->tim_hal->htimx->Init.RepetitionCounter = tim_dev->tim_cfg->repetcount;
    tim_dev->tim_hal->htimx->Init.AutoReloadPreload = tim_dev->tim_cfg->autoreload;

    if (HAL_TIM_Base_Init(tim_dev->tim_hal->htimx) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _tim_dev_open                                                    */
/* 功能描述: TIM定时器操作之开启                                             */
/* 输入参数: dev --- TIM定时器设备句柄                                       */
/*           flag --- 打开参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _tim_dev_open(struct device *dev, int flag)
{
    struct _tim_dev *tim_dev = (struct _tim_dev *)dev;
    assert_param(tim_dev);

    HAL_NVIC_EnableIRQ(tim_dev->tim_hal->irqx);
    __HAL_TIM_ENABLE(tim_dev->tim_hal->htimx);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _tim_dev_close                                                   */
/* 功能描述: TIM定时器操作之关闭                                             */
/* 输入参数: dev --- TIM定时器设备句柄                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _tim_dev_close(struct device *dev)
{
    struct _tim_dev *tim_dev = (struct _tim_dev *)dev;
    assert_param(tim_dev);

    HAL_NVIC_DisableIRQ(tim_dev->tim_hal->irqx);
    __HAL_TIM_DISABLE(tim_dev->tim_hal->htimx);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _tim_dev_control                                                 */
/* 功能描述: TIM定时器操作之控制                                             */
/* 输入参数: dev --- TIM定时器设备句柄                                       */
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
static int _tim_dev_control(struct device *dev, int cmd, void *args)
{
    struct _tim_dev *tim_dev = (struct _tim_dev *)dev;
    assert_param(tim_dev);

    switch(cmd)
    {
    case BSP_TIM_NEW_CYCLE:
        tim_dev->tim_cfg->period = *(int *)args - 1;
        _tim_dev_init(&tim_dev->tim_dev);
        break;
        
    default:
        break;
    }
    
    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_tim_init                                                     */
/* 功能描述: TIM定时器初始化和注册函数                                       */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_tim_init(void)
{
    const  char  *name = NULL;
    struct device *dev = NULL;

    for (int i = 0; i < sizeof(tim_devs) / sizeof(struct _tim_dev *); i++)
    {
        name = tim_devs[i]->tim_name;
        dev = &tim_devs[i]->tim_dev;

        assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
        strcpy(dev->name, name);

        dev->init    = _tim_dev_init;
        dev->open    = _tim_dev_open;
        dev->close   = _tim_dev_close;
        dev->control = _tim_dev_control;

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
INIT_BOARD_EXPORT(bsp_tim_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_tim_isr                                                      */
/* 功能描述: 定时器中断统一处理函数                                          */
/* 输入参数: index --- 定时器索引                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void bsp_tim_isr(enum _tim_dev_index index)
{
#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_enter(); 
#else
    __disable_irq();
#endif

    HAL_TIM_IRQHandler(tim_devs[index]->tim_hal->htimx);

#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_leave(); 
#else
    __enable_irq();
#endif
}

/*<FUNC+>**********************************************************************/
/* 函数名称: stm32_time_isr                                                   */
/* 功能描述: 定时器各中断具体处理函数                                        */
/* 输入参数: htim --- 定时器句柄                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {

    }
    else if (htim->Instance == TIM2)
    {

    }
    else if (htim->Instance == TIM3)
    {

    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: TIMx_IRQHandler                                                  */
/* 功能描述: TIM中断入口函数                                                  */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
#ifdef  BSP_USE_TIM1
void TIM1_CC_IRQHandler(void)
{
    bsp_tim_isr(TIM1_INDEX);
}
#endif

#ifdef  BSP_USE_TIM2
void TIM2_IRQHandler(void)
{
    bsp_tim_isr(TIM2_INDEX);
}
#endif

#ifdef  BSP_USE_TIM3
void TIM3_IRQHandler(void)
{
    bsp_tim_isr(TIM3_INDEX);
}
#endif

