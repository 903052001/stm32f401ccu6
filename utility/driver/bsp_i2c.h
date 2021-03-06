/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_i2c.h                                                        */
/* 内容摘要: I2C驱动初始化头文件                                              */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef BSP_I2C_H
#define BSP_I2C_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_device.h"

/******************************************************************************/
/*                              其他条件编译选项                             */
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
#define  BSP_USE_SOFT_I2C                              /* 使用软件模拟i2c   */
//#define  BSP_USE_HARD_I2C                              /* 使用硬件i2c控制器 */

#define  BSP_USE_I2C1
//#define  BSP_USE_I2C2
//#define  BSP_USE_I2C3

#define  BSP_I2C_WR               (1u << 0)             /* 写操作            */
#define  BSP_I2C_RD               (1u << 1)             /* 读操作            */ 
#define  BSP_I2C_NOSTART          (1u << 2)             /* 不需要新起始信号  */
#define  BSP_I2C_ADDR_10BIT       (1u << 3)             /* 10位设备地址      */

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
#pragma pack(1)

/*<STRUCT+>********************************************************************/
/* 结构: I2C_BUS_MSG                                                          */
/* 注释: I2C传输数据结构体                                                    */
/*<STRUCT->********************************************************************/
struct _i2c_bus_msg
{
    unsigned int         addr;                     /* 设备地址/数据子地址  */           
    unsigned char        flag;                     /* 操作标志: 读/写/ACK  */
    unsigned int         len;                      /* 读/写的数据长        */
    unsigned char       *buf;                      /* 读/写的数据头        */
};
typedef struct _i2c_bus_msg *i2c_bus_msg_t;

/*<STRUCT+>********************************************************************/
/* 结构: _I2C_BUS_CFG                                                         */
/* 注释: I2C总线配置参数                                                     */
/*<STRUCT->********************************************************************/
struct _i2c_bus_cfg
{   
#ifdef BSP_USE_HARD_I2C
    unsigned int         duty;                     /* I2C占空比             */
    unsigned int         baud;                     /* I2C波特率             */
    unsigned int         addrwidth;                /* I2C地址位宽,同一总线须挂相同地址位宽的外设 */
    unsigned int         datawidth;                /* I2C数据位宽,同一总线可挂不同数据位宽的外设 */
#endif

#ifdef BSP_USE_SOFT_I2C
    GPIO_TypeDef        *sda_port;                 /* SDA引脚端口号         */
    unsigned int         sda_pin;                  /* SDA引脚号             */
    GPIO_TypeDef        *scl_port;                 /* SCL引脚端口号         */
    unsigned int         scl_pin;                  /* SCL引脚号             */
#endif

    unsigned int         holdtime;                 /* 状态保持时间(单位/us),取决于外设 */
    unsigned int         wrcycle;                  /* 写周期时间(单位/ms),取决于外设   */
};
typedef struct _i2c_bus_cfg *i2c_bus_cfg_t;

/*<STRUCT+>********************************************************************/
/* 结构: _I2C_BUS_HAL                                                         */
/* 注释: I2C总线HAL库句柄                                                     */
/*<STRUCT->********************************************************************/
struct _i2c_bus_hal
{
    I2C_TypeDef         *i2cx;                     /* I2C号                 */
    IRQn_Type            irqx1;                    /* I2C中断号1            */
    IRQn_Type            irqx2;                    /* I2C中断号2            */
    I2C_HandleTypeDef   *hi2cx;                    /* I2C设备句柄           */
};
typedef struct _i2c_bus_hal *i2c_bus_hal_t;

/*<STRUCT+>********************************************************************/
/* 结构: _I2C_BUS_DEV                                                         */
/* 注释: I2C总线设备                                                          */
/*<STRUCT->********************************************************************/
struct _i2c_bus_dev
{
    struct device        i2c_dev;                  /* I2C总线设备           */
    const  char         *i2c_name;                 /* I2C总线名称           */
    i2c_bus_hal_t        i2c_hal;                  /* I2C总线句柄           */
    i2c_bus_cfg_t        i2c_cfg;                  /* I2C总线参数           */
};
typedef struct _i2c_bus_dev *i2c_bus_dev_t;

#pragma pack()

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int bsp_i2c_init(void);


#ifdef __cplusplus
}
#endif

#endif


