/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_fsm.h                                                         */
/* 内容摘要: 任务管理+定时器+虚拟状态机头文件                                */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-02-28                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-02-28        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_FSM_H
#define FN_FSM_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
/* C include */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include "stm32f4xx.h"

/* rtos include */
#include <board.h>
#include <watchdog.h>
#include <ulog.h>
#include <dfs.h>
#include <shell.h>
#include <at.h>

/* user drv include */
#include "IO_Macro.h"
#include "drv_io.h"
#include "drv_exbt.h"
#include "drv_gps.h"
#include "drv_exwdt.h"
#include "drv_simcom.h"
#include "proto_app.h"
#include "drv_w25qxx.h"
#include "history_flash.h"
#include "drv_imu.h"

/* user include */
#include "task_main.h"
#include "task_app.h"
#include "task_can.h"
#include "task_adc.h"
#include "task_imu.h"
#include "task_flash.h"
#include "task_gprs.h"
#include "task_gps.h"
#include "task_gpio.h"
#include "task_iwdg.h"
#include "task_led.h"
#include "task_dtu.h"
#include "task_exbt.h"

/* encode include */
#include "Fn_md5.h"
#include "Fn_aes.h"
#include "Fn_check.h"
#include "Fn_format.h"
#include "Fn_splitdata.h"

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
#define USE_YES               1              /* 任务开启                      */
#define USE_NO                0              /* 任务关闭                      */
#define RETRYS                5              /* 重复次数                      */
#define EXE_OK                0              /* 执行成功                      */
#define TIME_ZOOM         (60*8*60)          /* 东八区                        */

#define APP_VER              620             /* 程序版本号，每次升级必改此处 */
#define USE_TEST_SERVER       1              /* =1 测试服务器; =0 线上服务器 */
#define CAN_TEST_URL          0              /* =1 强制更新can配置           */
#define DBUG_WITHOUT_IWDG     0              /* =1 调试时关闭看门狗          */
#define USE_EncodeTrans       1              /* =1 开启加密传输              */
#define AES_LOCAL_KEY    "ifengniao"         /* AES加密本地密码              */
#define UP_JSON_TAIL      "{{{*}}}"          /* 上行补发数据尾               */

/******************************************************************************/
/* 公共异常码起始位置0x0000, 异常码范围:0x0000-0x03FF, 1K                    */
/******************************************************************************/
#define _ERR_BEGIN               ((uint16_t)0x0000)
#define _ERR_UNKNOWN             (_ERR_BEGIN + 0x01)    /* 未知错误          */
#define _VERSION_FAILED          (_ERR_BEGIN + 0x02)    /* 版本信息非法      */
#define _PARAM_FAILED            (_ERR_BEGIN + 0x03)    /* 输入参数非法      */
#define _POINTER_IS_NULL         (_ERR_BEGIN + 0x04)    /* 内存指针非法      */
#define _UNBOUND_ARRAY           (_ERR_BEGIN + 0x05)    /* 数组越界          */
#define _MALLOC_FAILED           (_ERR_BEGIN + 0x06)    /* 内存分配失败      */
#define _RELEASE_FAILED          (_ERR_BEGIN + 0x07)    /* 内存释放失败      */
#define _BUF_NOT_ENOUGH          (_ERR_BEGIN + 0x08)    /* 缓冲区不足        */
#define _UNSUPPORTED             (_ERR_BEGIN + 0x09)    /* 不支持的操作      */
#define _UNDEAL                  (_ERR_BEGIN + 0x0A)    /* 不处理的操作      */
#define _SEND_MSG_FAILED         (_ERR_BEGIN + 0x0B)    /* 消息发送失败      */
#define _RECV_MSG_FAILED         (_ERR_BEGIN + 0x0C)    /* 消息接收失败      */
#define _LIST_OPT_FAILED         (_ERR_BEGIN + 0x0D)    /* 链表操作失败      */
#define _MEM_ILLEGAL_MDY         (_ERR_BEGIN + 0x0E)    /* 内存已被非法修改  */
#define _TIMEOUT                 (_ERR_BEGIN + 0x0F)    /* 超时              */
#define _UNKNOWN_DATA            (_ERR_BEGIN + 0x10)    /* 不能识别的数据    */
#define _UNINIT                  (_ERR_BEGIN + 0x11)    /* 未初始化          */
#define _ERR_END                 ((uint16_t)0x03FF)     /* 错误结束          */
#define _OPER_TYPE_UNKNOWN       ((uint16_t)0xFFFF)     /* 未知操作类型      */

/******************************************************************************/
/* MSG_RETURN_CODE:响应消息的执行结果  结果码范围:0x00-0xFF                  */
/******************************************************************************/
#define MSG_SUCCESS              ((uint8_t)0x00)   /* 执行成功,用于应答      */
#define MSG_FAILURE              ((uint8_t)0x01)   /* 执行失败,用于应答      */
#define MSG_TIMEOUT              ((uint8_t)0x02)   /* 执行超时,用于超时应答  */
#define MSG_REPEATED             ((uint8_t)0x03)   /* 重复执行               */
#define MSG_VER_ERR              ((uint8_t)0x04)   /* 版本错误               */
#define MSG_BLE_SCAN_ERR         ((uint8_t)0x05)   /* 蓝牙扫描错误           */
#define MSG_UNKNOWN              ((uint8_t)0xFF)   /* 执行结果未知,用于请求  */

/******************************************************************************/
/* TImer_ID 定时器ID                                                          */
/******************************************************************************/
#define TIMER_ALL                ((uint8_t)0x00)    /* 所有定时器统一操作    */
#define TIMER_SENDOK             ((uint8_t)0x01)    /* 单次定时器            */
#define TIMER_EXWDT              ((uint8_t)0x02)    /* 外部看门狗喂狗定时器  */
#define TIMER_INWDT              ((uint8_t)0x03)    /* 内部看门狗喂狗定时器  */
#define TIMER_CANFD              ((uint8_t)0x04)    /* CANFD定时器           */
#define TIMER_SENDL              ((uint8_t)0x05)    /* 查发送链表定时器      */
#define TIMER_CEXBT              ((uint8_t)0x06)    /* 定时查蓝牙MAC         */
#define TIMER_ADC                ((uint8_t)0x07)    /* 定时查ADC             */
#define TIMER_LED                ((uint8_t)0x08)    /* 闪灯定时器            */
#define TIMER_RETCP              ((uint8_t)0x09)    /* 建立TCP定时器         */
#define TIMER_CHACC              ((uint8_t)0x0A)    /* 查询ACC定时器         */
#define TIMER_SINGLE             ((uint8_t)0x0B)    /* 通用单次定时器        */
#define TIMER_CMD41              ((uint8_t)0x0C)    /* 心跳定时器            */
#define TIMER_CMD42              ((uint8_t)0x0D)    /* 登陆上传定时器        */
#define TIMER_CMD43              ((uint8_t)0x0E)    /* 激活上传定时器        */
#if USE_EncodeTrans == 1
#define TIMER_CMD51              ((uint8_t)0x0F)    /* 多条包上传定时器      */
#define TIMER_CMD2D              ((uint8_t)0x10)    /* 混合包上传定时器      */
#define TIMER_NUMBER             ((uint8_t)0x10)    /* 定时器总数            */
#else
#define TIMER_CMD21              ((uint8_t)0x0F)    /* 状态上传定时器        */
#define TIMER_CMD22              ((uint8_t)0x10)    /* 定位上传定时器        */
#define TIMER_NUMBER             ((uint8_t)0x10)    /* 定时器总数            */
#endif

/******************************************************************************/
/* 任务ID: TASK_ID                                                            */
/******************************************************************************/
#define TASK_NUMBER              ((uint16_t)0x000D)     /* 任务总数          */
#define TASK_TIME                ((uint16_t)0x0000)     /* 定时器任务        */
#define TASK_MAIN                ((uint16_t)0x0100)     /* 系统主任务        */
#define TASK_APP                 ((uint16_t)0x0200)     /* APP任务           */
#define TASK_CAN                 ((uint16_t)0x0300)     /* CAN任务           */
#define TASK_ADC                 ((uint16_t)0x0400)     /* ADC任务           */
#define TASK_IMU                 ((uint16_t)0x0500)     /* IMU任务           */
#define TASK_FLASH               ((uint16_t)0x0600)     /* FLASH任务         */
#define TASK_GPRS                ((uint16_t)0x0700)     /* GPRS任务          */
#define TASK_GPS                 ((uint16_t)0x0800)     /* GPS任务           */
#define TASK_GPIO                ((uint16_t)0x0900)     /* GPIO任务          */
#define TASK_IWDG                ((uint16_t)0x0A00)     /* IWDG任务          */
#define TASK_LED                 ((uint16_t)0x0B00)     /* LED任务           */
#define TASK_DTU                 ((uint16_t)0x0C00)     /* DTU任务           */
#define TASK_EXBT                ((uint16_t)0x0D00)     /* EXBT任务          */
#define TASK_UNK                 ((uint16_t)0xFFFF)     /* 未知任务          */

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
typedef void (*Function_Entrance)(void *parameter);    /* 入口函数指针 */

#pragma pack(1)
/*<STRUCT+>********************************************************************/
/* 结构: Times                                                                */
/* 注释: 时间结构体                                                           */
/*<STRUCT->********************************************************************/
typedef struct times
{
    int Year;
    int Mon;
    int Day;
    int Hour;
    int Min;
    int Second;
} Times;

/*<STRUCT+>********************************************************************/
/* 结构: TASK_CLASS                                                           */
/* 注释: 任务信息结构体                                                       */
/*<STRUCT->********************************************************************/
typedef struct
{
    uint16_t           taskUse;              /* 是否启用任务的标志             */
    uint16_t           taskID;               /* 任务标识                       */
    Function_Entrance  taskEntry;            /* 任务入口函数                   */
    const char        *taskName;             /* 指向任务名称的指针             */
    uint16_t           taskStack;            /* 任务栈大小，以字节为单位       */
    void              *taskParam;            /* 指向任务参数的指针             */
    uint16_t           taskPrio;             /* 任务优先级                     */
    uint16_t           taskTick;             /* 任务时间片                     */
    rt_thread_t       *taskHandle;           /* 指向任务句柄的指针             */
    rt_mq_t           *queueHandle;          /* 指向消息队列句柄的指针         */
    const char        *queueName;            /* 消息队列名称                   */
    uint8_t           *queueCnt;             /* 消息队列使用情况               */
    uint16_t           queueDeep;            /* 消息队列成员总个数             */
    uint16_t           queueItem;            /* 消息队列单个成员大小           */
    uint16_t           queueFlag;            /* 消息队列出入对方式             */
} TASK_CLASS;

/*<STRUCT+>********************************************************************/
/* 结构: TIME_CLASS                                                           */
/* 注释: 通用定时器结构体                                                    */
/*<STRUCT->********************************************************************/
#pragma pack(1)
typedef struct
{
    uint8_t            timeUse;                 /* 初始化时是否启动标志位      */
    uint8_t            timeID;                  /* 定时器ID                    */
    const char        *timeName;                /* 定时器名字                  */
    uint32_t           timePeriod;              /* 定时器周期                  */
    rt_timer_t        *timeHandle;              /* 定时器操作句柄              */
    uint8_t            timeReload;              /* 周期/单次定时器             */
    Function_Entrance  timeCallback;            /* 定时器超时回调函数          */
} TIME_CLASS;

/*<STRUCT+>********************************************************************/
/* 结构: MSG_CLASS                                                            */
/* 注释: 消息结构                                                             */
/*<STRUCT->********************************************************************/
typedef struct
{
    uint16_t   receiver;            /* 消息接收者  one of: TASK_ID;            */
    uint16_t   sender;              /* 消息发送者  one of: TASK_ID;            */
    uint16_t   code;                /* 消息码(调用函数的ID)                    */
    uint16_t   cbcode;              /* 执行结果回调码(sender表)                */
    uint16_t   result;              /* 消息执行结果  one of: MSG_RETURN_CODE   */
    uint16_t   length;              /* 消息内容缓冲区的长度                    */
    void      *pvBuf;               /* 指向消息内容缓冲区的指针                */
} MSG_CLASS;

#pragma pack()

/*<STRUCT+>********************************************************************/
/* 结构: FUN_CLASS                                                            */
/* 注释: 命令码回调函数结构体                                                */
/*<STRUCT->********************************************************************/
typedef struct
{
    uint16_t            code;                          /* 操作命令码           */
    Function_Entrance   pfFUNc;                        /* 对应的回调执行函数   */
} FUN_CLASS;

/*<STRUCT+>********************************************************************/
/* 结构: RING_FIFO                                                            */
/* 注释: 数据环形缓存区结构体                                                */
/*<STRUCT->********************************************************************/
typedef struct ring_fifo
{
    rt_uint8_t   *pbuff;                               /* 缓存区地址           */
    rt_uint16_t   put;                                 /* 入队标识             */
    rt_uint16_t   get;                                 /* 出队标识             */
    rt_uint16_t   bufsz;                               /* 缓存区大小           */
} RING_FIFO;

/*<STRUCT+>********************************************************************/
/* 结构: LINK                                                                 */
/* 注释: 单向链表结构体                                                      */
/*<STRUCT->********************************************************************/
struct link
{
    rt_uint32_t   sernum;
    void         *data;
    struct link  *next;
};

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/
/******************************************************************************/
/* 软件定时器句柄                                                             */
/******************************************************************************/
#ifdef TIMER_SENDOK
extern rt_timer_t  TimerSendok_handle;
#endif
#ifdef TIMER_CMD41
extern rt_timer_t  TimerCmd41_handle;
#endif
#ifdef TIMER_CMD42
extern rt_timer_t  TimerCmd42_handle;
#endif
#ifdef TIMER_CMD43
extern rt_timer_t  TimerCmd43_handle;
#endif
#ifdef TIMER_CMD21
extern rt_timer_t  TimerCmd21_handle;
#endif
#ifdef TIMER_CMD22
extern rt_timer_t  TimerCmd22_handle;
#endif
#ifdef TIMER_CMD2D
extern rt_timer_t  TimerCmd2d_handle;
#endif
#ifdef TIMER_CMD51
extern rt_timer_t  TimerCmd51_handle;
#endif
#ifdef TIMER_EXWDT
extern rt_timer_t  TimerExwdt_handle;
#endif
#ifdef TIMER_INWDT
extern rt_timer_t  TimerInwdt_handle;
#endif
#ifdef TIMER_CANFD
extern rt_timer_t  TimerCanfd_handle;
#endif
#ifdef TIMER_SENDL
extern rt_timer_t  TimerSendl_handle;
#endif
#ifdef TIMER_CEXBT
extern rt_timer_t  TimerCexbt_handle;
#endif
#ifdef TIMER_ADC
extern rt_timer_t  TimerADC_handle;
#endif
#ifdef TIMER_LED
extern rt_timer_t  TimerLED_handle;
#endif
#ifdef TIMER_RETCP
extern rt_timer_t  TimerRetcp_handle;
#endif
#ifdef TIMER_CHACC
extern rt_timer_t  TimerChacc_handle;
#endif
#ifdef TIMER_SINGLE
extern rt_timer_t  TimerSingle_handle;
#endif

/******************************************************************************/
/* 任务和消息队列句柄列表                                                     */
/******************************************************************************/
extern TASK_CLASS gTaskMap[TASK_NUMBER];
#ifdef TASK_MAIN
extern rt_mq_t       main_mq;
extern rt_uint8_t    main_mq_cnt;
extern rt_thread_t   main_TaskHandle;
#endif

#ifdef TASK_APP
extern rt_mq_t       app_mq;
extern rt_uint8_t    app_mq_cnt;
extern rt_thread_t   app_TaskHandle;
#endif

#ifdef TASK_CAN
extern rt_mq_t       can_mq;
extern rt_uint8_t    can_mq_cnt;
extern rt_thread_t   can_TaskHandle;
#endif

#ifdef TASK_ADC
extern rt_mq_t       adc_mq;
extern rt_uint8_t    adc_mq_cnt;
extern rt_thread_t   adc_TaskHandle;
#endif

#ifdef TASK_IMU
extern rt_mq_t       imu_mq;
extern rt_uint8_t    imu_mq_cnt;
extern rt_thread_t   imu_TaskHandle;
#endif

#ifdef TASK_FLASH
extern rt_mq_t       flash_mq;
extern rt_uint8_t    flash_mq_cnt;
extern rt_thread_t   flash_TaskHandle;
#endif

#ifdef TASK_GPRS
extern rt_mq_t       gprs_mq;
extern rt_uint8_t    gprs_mq_cnt;
extern rt_thread_t   gprs_TaskHandle;
#endif

#ifdef TASK_GPS
extern rt_mq_t       gps_mq;
extern rt_uint8_t    gps_mq_cnt;
extern rt_thread_t   gps_TaskHandle;
#endif

#ifdef TASK_GPIO
extern rt_mq_t       gpio_mq;
extern rt_uint8_t    gpio_mq_cnt;
extern rt_thread_t   gpio_TaskHandle;
#endif

#ifdef TASK_LED
extern rt_mq_t       led_mq;
extern rt_uint8_t    led_mq_cnt;
extern rt_thread_t   led_TaskHandle;
#endif

#ifdef TASK_IWDG
extern rt_mq_t       iwdg_mq;
extern rt_uint8_t    iwdg_mq_cnt;
extern rt_thread_t   iwdg_TaskHandle;
#endif

#ifdef TASK_EXBT
extern rt_mq_t       exbt_mq;
extern rt_uint8_t    exbt_mq_cnt;
extern rt_thread_t   exbt_TaskHandle;
#endif

#ifdef TASK_DTU
extern rt_mq_t       dtu_mq;
extern rt_uint8_t    dtu_mq_cnt;
extern rt_thread_t   dtu_TaskHandle;
#endif

/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int Fsm_SendAsyncMsg(MSG_CLASS *ptMsg, uint8_t bUrgent);
int Fsm_RecvAsyncMsg(MSG_CLASS *ptMsg, int wait);
uint16_t Fsm_GetOperCode(void *ptMsg);
void Fsm_FunHandle(MSG_CLASS *ptMsg, FUN_CLASS **ptCurStat);
void Fsm_Task_SendAsyncMsg(uint16_t sender, uint16_t receiver, uint16_t code, uint16_t cbcode, uint16_t result, uint16_t len, uint8_t *pbuff, uint8_t urgent);
void Fsm_Task_RecvAsyncMsgHndl(MSG_CLASS *ptMsg, FUN_CLASS **fun_table);
void Fsm_SysReset(void);
void *Fsm_GetUB(uint32_t size);
void Fsm_RetUB(void *pBuf);
void Application_Init(void);
void Fsm_Task_StackState(MSG_CLASS *pMsg, TASK_CLASS *pTask);
TASK_CLASS *Fsm_GetAppTaskInfo(uint16_t usTaskId);
TASK_CLASS *Fsm_GetCurrAppTaskInfo(void);

/* 定时器相关 */
void Fsm_TimerCreate(uint8_t timeID);
void Fsm_TimerStart(uint8_t timeID);
void Fsm_TimerStop(uint8_t timeID);
void Fsm_TimerControl(uint8_t timeID, uint8_t cmd, void *arg);

/* 时间戳 */
Times stamp_to_standard(int stampTime);
int standard_to_stamp(char *str_time);

/* 链表操作函数 */
struct link *Link_insert_node_either(struct link *link_head, void *node_data, rt_uint32_t insert_pos, rt_uint8_t insert_dir);
struct link *Link_delete_node_either(struct link *link_head, int node_num);
struct link *Link_add_node_to_tail(struct link *link_head, void *node_data, char type);
struct link *Link_delete_all_node(struct link *link_head);


#ifdef __cplusplus
}
#endif

#endif



