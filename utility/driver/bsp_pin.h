/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_pin.h                                                        */
/* 内容摘要: 引脚驱动头文件                                                   */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-12-16                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-12-16        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef BSP_PIN_H
#define BSP_PIN_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_device.h"

/******************************************************************************/
/*                              其他条件编译选项                              */
/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                  常量定义                                  */
/******************************************************************************/


/******************************************************************************/
/*                                 全局宏定义                                 */
/******************************************************************************/
#define __STM32_PORT(port)   GPIO##port##_BASE

#define GET_PIN(PORTx, PIN)  (unsigned int)((16 * (((unsigned int)__STM32_PORT(PORTx) - (unsigned int)GPIOA_BASE) / (0x0400UL))) + PIN)

#define __STM32_PIN(index, gpio, gpio_index)                                \
    {                                                                       \
        index, GPIO##gpio, GPIO_PIN_##gpio_index                            \
    }

#define __STM32_PIN_RESERVE                                                 \
    {                                                                       \
        -1, 0, 0                                                            \
    }


#define PIN_LOW                         0x00              /* IO拉低 */
#define PIN_HIGH                        0x01              /* IO拉高 */

#define PIN_MODE_OUTPUT                 0x01              /* IO输出模式 */
#define PIN_MODE_INPUT                  0x02              /* IO输入模式 */
#define PIN_MODE_INPUT_PULLUP           0x03              /* IO上拉输入 */
#define PIN_MODE_INPUT_PULLDOWN         0x04              /* IO下拉输入 */
#define PIN_MODE_OUTPUT_OD              0x05              /* IO推挽输出 */
      
#define PIN_IRQ_MODE_RISING             0x11              /* 上升沿触发中断 */
#define PIN_IRQ_MODE_FALLING            0x12              /* 下降沿触发中断 */
#define PIN_IRQ_MODE_RISING_FALLING     0x13              /* 双边沿触发中断 */
#define PIN_IRQ_MODE_HIGH_LEVEL         0x14              /* 高电平沿触发中断,不支持 */
#define PIN_IRQ_MODE_LOW_LEVEL          0x15              /* 低电平沿触发中断,不支持 */

#define PWMOC_HALF_HZ                   20000
#define PWMOC_1_HZ                      10000
#define PWMOC_5_HZ                      2000
#define PWMOC_2_HZ                      4000
#define PWMOC_DUTY_50                   2
#define PWMOC_DUTY_75                   5

#define PIN_LED1                        GET_PIN(C, 13)    /* LED1 <--> C_13 */
#define PIN_LED2                        GET_PIN(C, 12)    /* LED2 <--> C_12 */
#define PIN_KEY1                        GET_PIN(B, 4)     /* KEY1 <--> B_4  */
#define PIN_KEY2                        GET_PIN(B, 5)     /* KEY2 <--> B_5  */
#define PIN_KEY3                        GET_PIN(B, 6)     /* KEY3 <--> B_6  */
#define PIN_KEY4                        GET_PIN(B, 7)     /* KEY4 <--> B_7  */

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
/*<STRUCT+>********************************************************************/
/* 结构: PIN_INDEX                                                            */
/* 注释: 引脚索引结构体                                                       */
/*<STRUCT->********************************************************************/
struct pin_index
{
    int               index;                      /* 索引序号 */
    GPIO_TypeDef     *gpio;                       /* 端口号   */
    int               pin;                        /* 引脚号   */
};

/*<STRUCT+>********************************************************************/
/* 结构: PIN_IRQ_MAP                                                          */
/* 注释: 引脚中断结构体                                                       */
/*<STRUCT->********************************************************************/
struct pin_irq_map
{
    int               pin;                        /* 引脚号 */
    int               priority;                   /* 优先级 */
    IRQn_Type         irqno;                      /* 中断号 */
};

/*<STRUCT+>********************************************************************/
/* 结构: PIN_IRQ_HDR                                                          */
/* 注释: 引脚中断回调结构体                                                  */
/*<STRUCT->********************************************************************/
struct pin_irq_hdr
{
    int               pin;                        /* 引脚号 */
    void (*hdr)(void *args);                      /* 回调函数 */
    void             *args;                       /* 回调参数 */
};

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int Fn_pin_read(int pin);
int Fn_pin_write(int pin, int val);
int Fn_pin_init(int pin, int mode);
int Fn_pin_deinit(int pin);
int Fn_pin_irq_init(int pin, int mode);
int Fn_pin_irq_deinit(int pin);
int bsp_pin_init(void);


#ifdef __cplusplus
    }
#endif

#endif


