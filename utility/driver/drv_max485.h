/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: drv_max485.h                                                    */
/* 内容摘要: MAX485芯片驱动头文件                                            */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-03-27                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-03-27        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef DRV_MAX485_H
#define DRV_MAX485_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "bsp_uart.h"

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
#define MAX485_USE_FLOW_CTRL                        /* MAX485是否启用流控  */     

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
#pragma pack(1)

/*<STRUCT+>********************************************************************/
/* 结构: _MAX485_PARA                                                        */
/* 注释: MAX485配置参数结构体                                                */
/*<STRUCT->********************************************************************/
struct _max485_para
{
    GPIO_TypeDef             *dir_port;             /* 读写方向引脚端口    */
    unsigned int              dir_pin;              /* 读写方向引脚        */
      signed int              fifosz;               /* FIFO数据区总大小    */
    struct _mfifo             fifo;                 /* 接收数据FIFO结构体  */
    
#ifdef  MAX485_USE_FLOW_CTRL
    GPIO_TypeDef             *cts_port;             /* _CTS端              */
    unsigned int              cts_pin;              /* _CTS脚              */
    GPIO_TypeDef             *rts_port;             /* _RTS端              */
    unsigned int              rts_pin;              /* _RTS脚              */
#endif 
};
typedef struct _max485_para *max485_para_t;

/*<STRUCT+>********************************************************************/
/* 结构: MAX485_DEVICE                                                       */
/* 注释: MAX485设备结构体                                                    */
/*<STRUCT->********************************************************************/
struct max485_device
{   
    struct device             dev;                   /* MAX485设备          */
    uart_bus_dev_t            uartx;                 /* UARTx总线设备       */
    struct _max485_para       para;                  /* MAX485配置参数      */
#ifdef  BSP_USE_RT_THREAD
    rt_thread_t               task;                  /* MAX485处理任务      */
    rt_mutex_t                lock;                  /* MAX485设备锁        */
    rt_sem_t                  rx_sem;                /* MAX485通知信号量    */
    rt_sem_t                  tx_sem;
#endif
};
typedef struct max485_device *max485_device_t;

#pragma pack()

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int bsp_max485_init(const char *name, const char *bus_name);


#ifdef __cplusplus
    }
#endif

#endif




