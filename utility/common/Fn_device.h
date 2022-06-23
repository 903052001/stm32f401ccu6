/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2021. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_device.h                                                      */
/* 内容摘要: 自定义设备驱动框架实现头文件                                     */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2021-01-07                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2021-01-07        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_DEVICE_H
#define FN_DEVICE_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_list.h"
#include "stm32f4xx.h"

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
//#define BSP_USE_RT_THREAD            /* 是否启用rt_thread */

#ifdef   BSP_USE_RT_THREAD

#include <rtthread.h>

#define dbug                         rt_kprintf

#else

#include "Fn_mvsprintf.h"

#define dbug                         mprintf

#define RT_USED                      __attribute__((used))
#define SECTION(x)                   __attribute__((section(x)))

#define INIT_EXPORT(fn, level)       RT_USED const init_fun __auto_init_##fn SECTION(".init_fun."level) = fn

#define INIT_PRE_EXPORT(fn)          INIT_EXPORT(fn, "1")   /* 预处理初始化 */   
#define INIT_BOARD_EXPORT(fn)        INIT_EXPORT(fn, "2")   /* 板级初始化   */
#define INIT_DEVICE_EXPORT(fn)       INIT_EXPORT(fn, "3")   /* 设备初始化   */
#define INIT_COMPONENT_EXPORT(fn)    INIT_EXPORT(fn, "4")   /* 组件初始化   */
#define INIT_ENV_EXPORT(fn)          INIT_EXPORT(fn, "5")   /* 环境初始化   */
#define INIT_APP_EXPORT(fn)          INIT_EXPORT(fn, "6")   /* 应用初始化   */

#endif

#define DEVICE_VERSION               ("1.0.0")    /* 设备框架版本号         */
#define DEVICE_NAME_MAXLEN           (16)         /* 设备名称字符串长度上限 */

#define TIMER_CIRCLE                 0x00         /* 周期定时模式           */
#define TIMER_SINGLE                 0x01         /* 单次定时模式           */
#define TIMER_DISABLE                0x00         /* 停用定时器             */
#define TIMER_ENABLE                 0x01         /* 启用定时器             */

#define TIMER_NEW_CYCLE              0x01         /* 定时器设备操作码之新周期值   */
#define TIMER_NEW_MODE               0x02         /* 定时器设备操作码之新工作模式 */
#define TIMER_NEW_STATE              0x03         /* 定时器设备操作码之是否启用   */

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
#pragma pack(1)

/*<STRUCT+>********************************************************************/
/* 结构: TIMER                                                                */
/* 注释: 软件定时器设备结构体                                                */
/*<STRUCT->********************************************************************/
struct timer
{
    unsigned char        isuse;                    /* 定时器是否启用(1:启用) */
    unsigned char        mode;                     /* 定时器模式(0:周期/1:单次) */
    unsigned char        index;                    /* 定时器设备号           */
    unsigned int         reload;                   /* 定时器超时时间值       */
    unsigned int         current;                  /* 定时器当前剩余时间值   */
    int                (*tmocb)(void *para);       /* 定时器超时回调         */
    T_LIST_NODE          node;                     /* 定时器设备链表节点     */
};
typedef struct timer * timer_t; 

/*<STRUCT+>********************************************************************/
/* 结构: DEVICE                                                               */
/* 注释: 设备结构体                                                           */
/*<STRUCT->********************************************************************/
struct device
{
    char         name[DEVICE_NAME_MAXLEN];   /* 设备名 */
    T_LIST_NODE  node;                       /* 设备链表节点 */
    void        *user;                       /* 设备私域数据 */

    /* 设备操作函数 */
    int (*init)(struct device *dev);
    int (*open)(struct device *dev, int flag);
    int (*close)(struct device *dev);
    int (*read)(struct device *dev, int pos, void *buffer, int size);
    int (*write)(struct device *dev, int pos, const void *buffer, int size);
    int (*control)(struct device *dev, int cmd, void *args);
    /* 设备接收回调和发送完成回调 */
    int (*rxind)(struct device *dev, int size);
    int (*txind)(struct device *dev, void *buffer);
    /* 总线传输函数 */
    int (*xfer)(struct device *dev, void *msgs, int num);
};
typedef struct device * device_t; 

typedef int (*rx_indi)(struct device *dev, int size);
typedef int (*tx_comp)(struct device *dev, void *buffer);
typedef int (*init_fun)(void);            /* 初始化函数指针 */

#pragma pack()

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/ 


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
void Fn_hw_us_delay(int nus);
int  Fn_device_version(void);

int  Fn_device_list_init(void);
int  Fn_device_auto_init(void);
int  Fn_device_init(struct device *dev);
int  Fn_device_open(struct device *dev, int flag);
int  Fn_device_close(struct device *dev);
int  Fn_device_read(struct device *dev, int pos, void *buffer, int size);
int  Fn_device_write(struct device *dev, int pos, const void *buffer, int size);
int  Fn_device_control(struct device *dev, int cmd, void *arg);
int  Fn_device_xfer(struct device *dev, void *msgs, int num);
int  Fn_device_register(struct device *dev, const char *name);
int  Fn_device_unregister(struct device *dev);
void Fn_device_set_rx_indicate(struct device *dev, rx_indi rxind);
void Fn_device_set_tx_complete(struct device *dev, tx_comp txdone);
struct device *Fn_device_find(const char *name);

int  Fn_timer_list_init(void);
int  Fn_timer_register(unsigned char index, unsigned int reload, int mode, int (*cb)(void *));
int  Fn_timer_unregister(unsigned char index);
int  Fn_timer_start(unsigned char index);
int  Fn_timer_close(unsigned char index);
int  Fn_timer_control(unsigned char index, int cmd, void *args);
int  Fn_timer_handle(void);
struct timer *Fn_timer_find(unsigned char index);

void Fn_panic(const char *fmt, ...);
void Fn_hexdump(const void *p, unsigned int size);
void Fn_print_echo_hex(const char *name, int width, const char *buf, int size);
void Fn_print_Progressbar(const char *index, unsigned int currentLen, unsigned int totalLen);
int  Fn_printbuffer(unsigned long addr, char *data, unsigned int width, unsigned int count, unsigned int linelen);


#ifdef __cplusplus
    }
#endif

#endif


