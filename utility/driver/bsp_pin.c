/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_pin.c                                                        */
/* 内容摘要: 引脚驱动源文件                                                   */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-12-16                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-12-16        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <stdlib.h>
#include "bsp_pin.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define ITEM_NUM(items)    (sizeof(items) / sizeof(items[0]))    /* 成员个数 */

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static const struct pin_index *get_pin(int pin);
static void _pin_clock(const struct pin_index *index);

static void HAL_EXIT0_Handle(void *args);
static void HAL_EXIT1_Handle(void *args);
static void HAL_EXIT2_Handle(void *args);
static void HAL_EXIT3_Handle(void *args);
static void HAL_EXIT4_Handle(void *args);
static void HAL_EXIT5_Handle(void *args);
static void HAL_EXIT6_Handle(void *args);
static void HAL_EXIT7_Handle(void *args);
static void HAL_EXIT8_Handle(void *args);
static void HAL_EXIT9_Handle(void *args);
static void HAL_EXIT10_Handle(void *args);
static void HAL_EXIT11_Handle(void *args);
static void HAL_EXIT12_Handle(void *args);
static void HAL_EXIT13_Handle(void *args);
static void HAL_EXIT14_Handle(void *args);
static void HAL_EXIT15_Handle(void *args);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static const struct pin_index pins[] =
{
#if defined(GPIOA)
    __STM32_PIN(0,   A, 0),
    __STM32_PIN(1,   A, 1),
    __STM32_PIN(2,   A, 2),
    __STM32_PIN(3,   A, 3),
    __STM32_PIN(4,   A, 4),
    __STM32_PIN(5,   A, 5),
    __STM32_PIN(6,   A, 6),
    __STM32_PIN(7,   A, 7),
    __STM32_PIN(8,   A, 8),
    __STM32_PIN(9,   A, 9),
    __STM32_PIN(10,  A, 10),
    __STM32_PIN(11,  A, 11),
    __STM32_PIN(12,  A, 12),
    __STM32_PIN(13,  A, 13),
    __STM32_PIN(14,  A, 14),
    __STM32_PIN(15,  A, 15),
#endif

#if defined(GPIOB)
    __STM32_PIN(16,  B, 0),
    __STM32_PIN(17,  B, 1),
    __STM32_PIN(18,  B, 2),
    __STM32_PIN(19,  B, 3),
    __STM32_PIN(20,  B, 4),
    __STM32_PIN(21,  B, 5),
    __STM32_PIN(22,  B, 6),
    __STM32_PIN(23,  B, 7),
    __STM32_PIN(24,  B, 8),
    __STM32_PIN(25,  B, 9),
    __STM32_PIN(26,  B, 10),
    __STM32_PIN(27,  B, 11),
    __STM32_PIN(28,  B, 12),
    __STM32_PIN(29,  B, 13),
    __STM32_PIN(30,  B, 14),
    __STM32_PIN(31,  B, 15),
#endif

#if defined(GPIOC)
    __STM32_PIN(32,  C, 0),
    __STM32_PIN(33,  C, 1),
    __STM32_PIN(34,  C, 2),
    __STM32_PIN(35,  C, 3),
    __STM32_PIN(36,  C, 4),
    __STM32_PIN(37,  C, 5),
    __STM32_PIN(38,  C, 6),
    __STM32_PIN(39,  C, 7),
    __STM32_PIN(40,  C, 8),
    __STM32_PIN(41,  C, 9),
    __STM32_PIN(42,  C, 10),
    __STM32_PIN(43,  C, 11),
    __STM32_PIN(44,  C, 12),
    __STM32_PIN(45,  C, 13),
    __STM32_PIN(46,  C, 14),
    __STM32_PIN(47,  C, 15),
#endif

#if defined(GPIOD)
    __STM32_PIN(48,  D, 0),
    __STM32_PIN(49,  D, 1),
    __STM32_PIN(50,  D, 2),
    __STM32_PIN(51,  D, 3),
    __STM32_PIN(52,  D, 4),
    __STM32_PIN(53,  D, 5),
    __STM32_PIN(54,  D, 6),
    __STM32_PIN(55,  D, 7),
    __STM32_PIN(56,  D, 8),
    __STM32_PIN(57,  D, 9),
    __STM32_PIN(58,  D, 10),
    __STM32_PIN(59,  D, 11),
    __STM32_PIN(60,  D, 12),
    __STM32_PIN(61,  D, 13),
    __STM32_PIN(62,  D, 14),
    __STM32_PIN(63,  D, 15),
#endif

#if defined(GPIOE)
    __STM32_PIN(64,  E, 0),
    __STM32_PIN(65,  E, 1),
    __STM32_PIN(66,  E, 2),
    __STM32_PIN(67,  E, 3),
    __STM32_PIN(68,  E, 4),
    __STM32_PIN(69,  E, 5),
    __STM32_PIN(70,  E, 6),
    __STM32_PIN(71,  E, 7),
    __STM32_PIN(72,  E, 8),
    __STM32_PIN(73,  E, 9),
    __STM32_PIN(74,  E, 10),
    __STM32_PIN(75,  E, 11),
    __STM32_PIN(76,  E, 12),
    __STM32_PIN(77,  E, 13),
    __STM32_PIN(78,  E, 14),
    __STM32_PIN(79,  E, 15),
#endif

#if defined(GPIOF)
    __STM32_PIN(80,  F, 0),
    __STM32_PIN(81,  F, 1),
    __STM32_PIN(82,  F, 2),
    __STM32_PIN(83,  F, 3),
    __STM32_PIN(84,  F, 4),
    __STM32_PIN(85,  F, 5),
    __STM32_PIN(86,  F, 6),
    __STM32_PIN(87,  F, 7),
    __STM32_PIN(88,  F, 8),
    __STM32_PIN(89,  F, 9),
    __STM32_PIN(90,  F, 10),
    __STM32_PIN(91,  F, 11),
    __STM32_PIN(92,  F, 12),
    __STM32_PIN(93,  F, 13),
    __STM32_PIN(94,  F, 14),
    __STM32_PIN(95,  F, 15),
#endif

#if defined(GPIOG)
    __STM32_PIN(96,  G, 0),
    __STM32_PIN(97,  G, 1),
    __STM32_PIN(98,  G, 2),
    __STM32_PIN(99,  G, 3),
    __STM32_PIN(100, G, 4),
    __STM32_PIN(101, G, 5),
    __STM32_PIN(102, G, 6),
    __STM32_PIN(103, G, 7),
    __STM32_PIN(104, G, 8),
    __STM32_PIN(105, G, 9),
    __STM32_PIN(106, G, 10),
    __STM32_PIN(107, G, 11),
    __STM32_PIN(108, G, 12),
    __STM32_PIN(109, G, 13),
    __STM32_PIN(110, G, 14),
    __STM32_PIN(111, G, 15),
#endif

#if defined(GPIOH)
    __STM32_PIN(112, H, 0),
    __STM32_PIN(113, H, 1),
    __STM32_PIN(114, H, 2),
    __STM32_PIN(115, H, 3),
    __STM32_PIN(116, H, 4),
    __STM32_PIN(117, H, 5),
    __STM32_PIN(118, H, 6),
    __STM32_PIN(119, H, 7),
    __STM32_PIN(120, H, 8),
    __STM32_PIN(121, H, 9),
    __STM32_PIN(122, H, 10),
    __STM32_PIN(123, H, 11),
    __STM32_PIN(124, H, 12),
    __STM32_PIN(125, H, 13),
    __STM32_PIN(126, H, 14),
    __STM32_PIN(127, H, 15),
#endif

#if defined(GPIOI)
    __STM32_PIN(128, I, 0),
    __STM32_PIN(129, I, 1),
    __STM32_PIN(130, I, 2),
    __STM32_PIN(131, I, 3),
    __STM32_PIN(132, I, 4),
    __STM32_PIN(133, I, 5),
    __STM32_PIN(134, I, 6),
    __STM32_PIN(135, I, 7),
    __STM32_PIN(136, I, 8),
    __STM32_PIN(137, I, 9),
    __STM32_PIN(138, I, 10),
    __STM32_PIN(139, I, 11),
    __STM32_PIN(140, I, 12),
    __STM32_PIN(141, I, 13),
    __STM32_PIN(142, I, 14),
    __STM32_PIN(143, I, 15),
#endif

#if defined(GPIOJ)
    __STM32_PIN(144, J, 0),
    __STM32_PIN(145, J, 1),
    __STM32_PIN(146, J, 2),
    __STM32_PIN(147, J, 3),
    __STM32_PIN(148, J, 4),
    __STM32_PIN(149, J, 5),
    __STM32_PIN(150, J, 6),
    __STM32_PIN(151, J, 7),
    __STM32_PIN(152, J, 8),
    __STM32_PIN(153, J, 9),
    __STM32_PIN(154, J, 10),
    __STM32_PIN(155, J, 11),
    __STM32_PIN(156, J, 12),
    __STM32_PIN(157, J, 13),
    __STM32_PIN(158, J, 14),
    __STM32_PIN(159, J, 15),
#endif

#if defined(GPIOK)
    __STM32_PIN(160, K, 0),
    __STM32_PIN(161, K, 1),
    __STM32_PIN(162, K, 2),
    __STM32_PIN(163, K, 3),
    __STM32_PIN(164, K, 4),
    __STM32_PIN(165, K, 5),
    __STM32_PIN(166, K, 6),
    __STM32_PIN(167, K, 7),
    __STM32_PIN(168, K, 8),
    __STM32_PIN(169, K, 9),
    __STM32_PIN(170, K, 10),
    __STM32_PIN(171, K, 11),
    __STM32_PIN(172, K, 12),
    __STM32_PIN(173, K, 13),
    __STM32_PIN(174, K, 14),
    __STM32_PIN(175, K, 15),
#endif
};

static const struct pin_irq_map pin_irq_maps[] =
{
#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32L0) || defined(SOC_SERIES_STM32G0)
    { GPIO_PIN_0,    10,    EXTI0_1_IRQn   },
    { GPIO_PIN_1,    10,    EXTI0_1_IRQn   },
    { GPIO_PIN_2,    10,    EXTI2_3_IRQn   },
    { GPIO_PIN_3,    10,    EXTI2_3_IRQn   },
    { GPIO_PIN_4,    10,    EXTI4_15_IRQn  },
    { GPIO_PIN_5,    10,    EXTI4_15_IRQn  },
    { GPIO_PIN_6,    10,    EXTI4_15_IRQn  },
    { GPIO_PIN_7,    10,    EXTI4_15_IRQn  },
    { GPIO_PIN_8,    10,    EXTI4_15_IRQn  },
    { GPIO_PIN_9,    10,    EXTI4_15_IRQn  },
    { GPIO_PIN_10,   10,    EXTI4_15_IRQn  },
    { GPIO_PIN_11,   10,    EXTI4_15_IRQn  },
    { GPIO_PIN_12,   10,    EXTI4_15_IRQn  },
    { GPIO_PIN_13,   10,    EXTI4_15_IRQn  },
    { GPIO_PIN_14,   10,    EXTI4_15_IRQn  },
    { GPIO_PIN_15,   10,    EXTI4_15_IRQn  },
#else
    { GPIO_PIN_0,    10,    EXTI0_IRQn     }, 
    { GPIO_PIN_1,    10,    EXTI1_IRQn     },
    { GPIO_PIN_2,    10,    EXTI2_IRQn     },
    { GPIO_PIN_3,    10,    EXTI3_IRQn     },
    { GPIO_PIN_4,    10,    EXTI4_IRQn     },
    { GPIO_PIN_5,    10,    EXTI9_5_IRQn   },
    { GPIO_PIN_6,    10,    EXTI9_5_IRQn   },
    { GPIO_PIN_7,    10,    EXTI9_5_IRQn   },
    { GPIO_PIN_8,    10,    EXTI9_5_IRQn   },
    { GPIO_PIN_9,    10,    EXTI9_5_IRQn   },
    { GPIO_PIN_10,   10,    EXTI15_10_IRQn },
    { GPIO_PIN_11,   10,    EXTI15_10_IRQn },
    { GPIO_PIN_12,   10,    EXTI15_10_IRQn },
    { GPIO_PIN_13,   10,    EXTI15_10_IRQn },
    { GPIO_PIN_14,   10,    EXTI15_10_IRQn },
    { GPIO_PIN_15,   10,    EXTI15_10_IRQn },
#endif
};

static struct pin_irq_hdr pin_irq_hdrs[] =
{
    { GPIO_PIN_0,   HAL_EXIT0_Handle,   NULL },
    { GPIO_PIN_1,   HAL_EXIT1_Handle,   NULL },
    { GPIO_PIN_2,   HAL_EXIT2_Handle,   NULL },
    { GPIO_PIN_3,   HAL_EXIT3_Handle,   NULL },
    { GPIO_PIN_4,   HAL_EXIT4_Handle,   NULL },
    { GPIO_PIN_5,   HAL_EXIT5_Handle,   NULL },
    { GPIO_PIN_6,   HAL_EXIT6_Handle,   NULL },
    { GPIO_PIN_7,   HAL_EXIT7_Handle,   NULL },
    { GPIO_PIN_8,   HAL_EXIT8_Handle,   NULL },
    { GPIO_PIN_9,   HAL_EXIT9_Handle,   NULL },
    { GPIO_PIN_10,  HAL_EXIT10_Handle,  NULL },
    { GPIO_PIN_11,  HAL_EXIT11_Handle,  NULL },
    { GPIO_PIN_12,  HAL_EXIT12_Handle,  NULL },
    { GPIO_PIN_13,  HAL_EXIT13_Handle,  NULL },
    { GPIO_PIN_14,  HAL_EXIT14_Handle,  NULL },
    { GPIO_PIN_15,  HAL_EXIT15_Handle,  NULL },
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: get_pin                                                          */
/* 功能描述: 通过GET_PIN(A,0)获取GPIOA_PIN_0                                  */
/* 输入参数: pin --- 引脚号，格式GET_PIN(A,0)                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: static const struct pin_index                                    */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static const struct pin_index *get_pin(int pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];

        if (index->index < 0)
        {
            index = NULL;
        }
    }
    else
    {
        index = NULL;
    }

    return index;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _pin_clock                                                       */
/* 功能描述: 开启引脚时钟                                                     */
/* 输入参数: index --- 引脚索引                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void _pin_clock(const struct pin_index *index)
{
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    if (index->gpio == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    if (index->gpio == GPIOB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    if (index->gpio == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    if (index->gpio == GPIOD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    if (index->gpio == GPIOE)
        __HAL_RCC_GPIOE_CLK_ENABLE();

#endif

#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    if (index->gpio == GPIOF)
        __HAL_RCC_GPIOF_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
    if (index->gpio == GPIOG)
    {
#ifdef SOC_SERIES_STM32L4
        HAL_PWREx_EnableVddIO2();
#endif
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
#endif

#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    if (index->gpio == GPIOH)
        __HAL_RCC_GPIOH_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    if (index->gpio == GPIOI)
        __HAL_RCC_GPIOI_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    if (index->gpio == GPIOJ)
        __HAL_RCC_GPIOJ_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    if (index->gpio == GPIOK)
        __HAL_RCC_GPIOK_CLK_ENABLE();
#endif
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_pin_read                                                      */
/* 功能描述: 读取某引脚输入电平                                               */
/* 输入参数: pin --- 引脚索引                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_pin_read(int pin)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == NULL)
    {
        return -1;
    }

    return HAL_GPIO_ReadPin(index->gpio, index->pin);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_pin_write                                                     */
/* 功能描述: 从某引脚输出电平                                                 */
/* 输入参数: pin --- 引脚索引                                                 */
/*           val --- 高/低电平                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_pin_write(int pin, int val)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == NULL)
    {
        return -1;
    }

    HAL_GPIO_WritePin(index->gpio, index->pin, (GPIO_PinState)val);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_pin_init                                                      */
/* 功能描述: 非中断方式使能引脚                                              */
/* 输入参数: pin --- 引脚索引                                                 */
/*           mode --- 引脚模式                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_pin_init(int pin, int mode)
{
    const struct pin_index *index;
    GPIO_InitTypeDef GPIO_InitStruct;

    index = get_pin(pin);
    if (index == NULL)
    {
        return -1;
    }

    /* Open GPIO_Clock */
    _pin_clock(index);

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin = index->pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    switch (mode)
    {
    case PIN_MODE_OUTPUT:
        /* output setting */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

    case PIN_MODE_INPUT:
        /* input setting: not pull. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

    case PIN_MODE_INPUT_PULLUP:
        /* input setting: pull up. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;

    case PIN_MODE_INPUT_PULLDOWN:
        /* input setting: pull down. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;

    case PIN_MODE_OUTPUT_OD:
        /* output setting: od. */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

    default:
        /* illegal mode */
        assert_param(0);
        break;
    }

    HAL_GPIO_Init(index->gpio, &GPIO_InitStruct);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_pin_deinit                                                    */
/* 功能描述: 非中断方式失能引脚                                               */
/* 输入参数: pin --- 引脚索引                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_pin_deinit(int pin)
{
    const struct pin_index *index;
    
    index = get_pin(pin);
    if (index == NULL)
    {
        return -1;
    }
    
    HAL_GPIO_DeInit(index->gpio, index->pin);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_pin_irq_init                                                  */
/* 功能描述: 中断方式使能引脚                                                 */
/* 输入参数: pin --- 引脚索引                                                 */
/*           mode --- 引脚中断模式                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_pin_irq_init(int pin, int mode)
{
    int   i;
    const struct pin_index *index;
    GPIO_InitTypeDef GPIO_InitStruct;

    index = get_pin(pin);
    if (index == NULL)
    {
        return -1;
    }

    /* Open GPIO_Clock */
    _pin_clock(index);

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin = index->pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    switch (mode)
    {
    case PIN_IRQ_MODE_RISING:
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        break;

    case PIN_IRQ_MODE_FALLING:
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        break;

    case PIN_IRQ_MODE_RISING_FALLING:
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        break;

    default:
        assert_param(0);
        break;
    }

    for (i = 0; i < ITEM_NUM(pin_irq_maps); i++)
    {
        if (index->pin == pin_irq_maps[i].pin)
        {
            HAL_GPIO_Init(index->gpio, &GPIO_InitStruct);
            HAL_NVIC_SetPriority(pin_irq_maps[i].irqno, pin_irq_maps[i].priority, 0);
            HAL_NVIC_EnableIRQ(pin_irq_maps[i].irqno);
            break;
        }
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_pin_irq_deinit                                                */
/* 功能描述: 中断方式失能引脚                                                 */
/* 输入参数: pin --- 引脚索引                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_pin_irq_deinit(int pin)
{
    int   i;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == NULL)
    {
        return -1;
    }
    
    for (i = 0; i < ITEM_NUM(pin_irq_maps); i++)
    {
        if (index->pin == pin_irq_maps[i].pin)
        {
            HAL_NVIC_DisableIRQ(pin_irq_maps[i].irqno);
            HAL_GPIO_DeInit(index->gpio, index->pin);
            break;
        }
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: HAL_GPIO_EXTI_Callback                                           */
/* 功能描述: 具体引脚中断回调处理                                             */
/* 输入参数: GPIO_Pin --- 引脚号                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void HAL_GPIO_EXTI_Callback(unsigned short GPIO_Pin)
{
    int i;
  
#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_enter(); 
#else
    __disable_irq();
#endif

    for (i = 0; i < ITEM_NUM(pin_irq_hdrs); i++)
    {
        if (GPIO_Pin == pin_irq_hdrs[i].pin)
        {
            pin_irq_hdrs[i].hdr(pin_irq_hdrs[i].args);
            break;
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
/* 函数名称: bsp_pin_init                                                     */
/* 功能描述: 板级IO初始化                                                     */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 按键+LED                                                         */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_pin_init(void)
{
    Fn_pin_init(PIN_LED1, PIN_MODE_OUTPUT);
    Fn_pin_init(PIN_LED2, PIN_MODE_OUTPUT);
    
    Fn_pin_irq_init(PIN_KEY1, PIN_IRQ_MODE_RISING);
    Fn_pin_irq_init(PIN_KEY2, PIN_IRQ_MODE_FALLING);
    Fn_pin_irq_init(PIN_KEY3, PIN_IRQ_MODE_RISING_FALLING);
    Fn_pin_irq_init(PIN_KEY4, PIN_IRQ_MODE_RISING_FALLING);
    
    return 0;
}

int _bsp_pin_init(void)
{
    return bsp_pin_init();
}
INIT_BOARD_EXPORT(_bsp_pin_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: 各引脚中断处理函数                                              */
/*<FUNC->**********************************************************************/
static void HAL_EXIT0_Handle(void *args)
{
    return;
}

static void HAL_EXIT1_Handle(void *args)
{
    return;
}

static void HAL_EXIT2_Handle(void *args)
{
    return;
}

static void HAL_EXIT3_Handle(void *args)
{
    return;
}

static void HAL_EXIT4_Handle(void *args)
{
    return;
}

static void HAL_EXIT5_Handle(void *args)
{
    return;
}

static void HAL_EXIT6_Handle(void *args)
{
    return;
}

static void HAL_EXIT7_Handle(void *args)
{
    return;
}

static void HAL_EXIT8_Handle(void *args)
{
    return;
}

static void HAL_EXIT9_Handle(void *args)
{
    return;
}

static void HAL_EXIT10_Handle(void *args)
{
    return;
}

static void HAL_EXIT11_Handle(void *args)
{
    return;
}

static void HAL_EXIT12_Handle(void *args)
{
    return;
}

static void HAL_EXIT13_Handle(void *args)
{
    return;
}

static void HAL_EXIT14_Handle(void *args)
{
    return;
}

static void HAL_EXIT15_Handle(void *args)
{
    return;
}


#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32L0)
void EXTI0_1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

#else

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}
#endif

