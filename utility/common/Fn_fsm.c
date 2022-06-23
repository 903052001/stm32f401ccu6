/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_fsm.c                                                         */
/* 内容摘要: 任务管理+定时器+虚拟状态机源文件                                */
/* 其它说明: 核心是虚拟状态机(fsm)                                           */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-02-28                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-02-28        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/


/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_fsm.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#ifdef __GNUC__
    #define CONSOLE UART5
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define CONSOLE UART5
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    CONSOLE->SR;
    CONSOLE->DR = (unsigned char)ch;
    while (!(CONSOLE->SR & 0x80));
    return 0;
}

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static void Fsm_AppTaskCreate(void);
static void Fsm_CheckAppTaskConfig(void);
static void Fsm_Appinit_Task(void *parameter);

#ifdef TIMER_EXWDT
    static void Timeout_Exwdt_Callback(void *parameter);
#endif
#ifdef TIMER_INWDT
    static void Timeout_Inwdt_Callback(void *parameter);
#endif
#ifdef TIMER_CANFD
    static void Timeout_Check_CanData_Callback(void *parameter);
#endif
#ifdef TIMER_CEXBT
    static void Timeout_Cexbt_Callback(void *parameter);
#endif
#ifdef TIMER_SENDL
    static void Timeout_SendLink_Callback(void *parameter);
#endif
#ifdef TIMER_SENDOK
    static void Timeout_Sendok_Callback(void *parameter);
#endif
#ifdef TIMER_ADC
    static void Timeout_ADC_Callback(void *parameter);
#endif
#ifdef TIMER_LED
    static void Timeout_LED_Callback(void *parameter);
#endif
#ifdef TIMER_RETCP
    static void Timeout_Retcp_Callback(void *parameter);
#endif
#ifdef TIMER_CHACC
    static void Timeout_Check_ACC_Callback(void *parameter);
#endif
#ifdef TIMER_SINGLE
    static void Timeout_Single_Callback(void *parameter);
#endif
#ifdef TIMER_CMD21
    static void Timeout_Cmd21_Callback(void *parameter);
#endif
#ifdef TIMER_CMD22
    static void Timeout_Cmd22_Callback(void *parameter);
#endif
#ifdef TIMER_CMD2D
    static void Timeout_Cmd2d_Callback(void *parameter);
#endif
#ifdef TIMER_CMD41
    static void Timeout_Cmd41_Callback(void *parameter);
#endif
#ifdef TIMER_CMD42
    static void Timeout_Cmd42_Callback(void *parameter);
#endif
#ifdef TIMER_CMD43
    static void Timeout_Cmd43_Callback(void *parameter);
#endif
#ifdef TIMER_CMD51
    static void Timeout_Cmd51_Callback(void *parameter);
#endif

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
/******************************************************************************/
/* 软件定时器句柄                                                             */
/******************************************************************************/
#ifdef TIMER_SENDOK
    rt_timer_t  TimerSendok_handle = NULL;
#endif
#ifdef TIMER_CMD41
    rt_timer_t  TimerCmd41_handle = NULL;
#endif
#ifdef TIMER_CMD42
    rt_timer_t  TimerCmd42_handle = NULL;
#endif
#ifdef TIMER_CMD43
    rt_timer_t  TimerCmd43_handle = NULL;
#endif
#ifdef TIMER_CMD2D
    rt_timer_t  TimerCmd2d_handle = NULL;
#endif
#ifdef TIMER_CMD51
    rt_timer_t  TimerCmd51_handle = NULL;
#endif
#ifdef TIMER_CMD21
    rt_timer_t  TimerCmd21_handle = NULL;
#endif
#ifdef TIMER_CMD22
    rt_timer_t  TimerCmd22_handle = NULL;
#endif
#ifdef TIMER_EXWDT
    rt_timer_t  TimerExwdt_handle = NULL;
#endif
#ifdef TIMER_INWDT
    rt_timer_t  TimerInwdt_handle = NULL;
#endif
#ifdef TIMER_CANFD
    rt_timer_t  TimerCanfd_handle = NULL;
#endif
#ifdef TIMER_SENDL
    rt_timer_t  TimerSendl_handle = NULL;
#endif
#ifdef TIMER_CEXBT
    rt_timer_t  TimerCexbt_handle = NULL;
#endif
#ifdef TIMER_ADC
    rt_timer_t  TimerADC_handle = NULL;
#endif
#ifdef TIMER_LED
    rt_timer_t  TimerLED_handle = NULL;
#endif
#ifdef TIMER_RETCP
    rt_timer_t  TimerRetcp_handle = NULL;
#endif
#ifdef TIMER_CHACC
    rt_timer_t  TimerChacc_handle = NULL;
#endif
#ifdef TIMER_SINGLE
    rt_timer_t  TimerSingle_handle = NULL;
#endif

/******************************************************************************/
/* 定时器表                                                                   */
/******************************************************************************/
TIME_CLASS gTimerMap[TIMER_NUMBER] =
{
#ifdef TIMER_SENDOK
    /****************************************************************************/
    /* 单次定时器,等待模组发数成功后回传SEND OK的时间(35s)                     */
    /****************************************************************************/
    {USE_YES, TIMER_SENDOK, "timer_sendok", 35000, &TimerSendok_handle, RT_TIMER_FLAG_ONE_SHOT, Timeout_Sendok_Callback},
#endif

#ifdef TIMER_CMD41
    /****************************************************************************/
    /* CMD41定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD41, "timer_cmd41",   15000, &TimerCmd41_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd41_Callback},
#endif

#ifdef TIMER_CMD42
    /****************************************************************************/
    /* CMD42定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD42,  "timer_cmd42",  15000, &TimerCmd42_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd42_Callback},
#endif

#ifdef TIMER_CMD43
    /****************************************************************************/
    /* CMD43定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD43,  "timer_cmd43",  15000, &TimerCmd43_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd43_Callback},
#endif

#ifdef TIMER_CMD21
    /****************************************************************************/
    /* CMD21定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD21, "timer_cmd21",   60000, &TimerCmd21_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd21_Callback},
#endif

#ifdef TIMER_CMD22
    /****************************************************************************/
    /* CMD22定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD22, "timer_cmd22",   60000, &TimerCmd22_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd22_Callback},
#endif

#ifdef TIMER_CMD2D
    /****************************************************************************/
    /* CMD2D定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD2D, "timer_cmd2d",   60000, &TimerCmd2d_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd2d_Callback},
#endif

#ifdef TIMER_CMD51
    /****************************************************************************/
    /* CMD51定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CMD51, "timer_cmd51",   60000, &TimerCmd51_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cmd51_Callback},
#endif

#ifdef TIMER_SENDL
    /****************************************************************************/
    /* 查发送链表定时器                                                        */
    /****************************************************************************/
    {USE_YES, TIMER_SENDL, "timer_sendl",    1000, &TimerSendl_handle, RT_TIMER_FLAG_PERIODIC, Timeout_SendLink_Callback},
#endif

#ifdef TIMER_EXWDT
    /****************************************************************************/
    /* 外部看门狗喂狗定时器                                                    */
    /****************************************************************************/
    {USE_YES, TIMER_EXWDT, "timer_exwdt",     500, &TimerExwdt_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Exwdt_Callback},
#endif

#ifdef TIMER_INWDT
    /****************************************************************************/
    /* 内部看门狗喂狗定时器                                                    */
    /****************************************************************************/
    {USE_YES, TIMER_INWDT, "timer_inwdt",    5000, &TimerInwdt_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Inwdt_Callback},
#endif

#ifdef TIMER_CANFD
    /****************************************************************************/
    /* CANFD定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_CANFD, "timer_canfd",    3000, &TimerCanfd_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Check_CanData_Callback},
#endif

#ifdef TIMER_CEXBT
    /****************************************************************************/
    /* 查蓝牙MAC定时器                                                         */
    /****************************************************************************/
    {USE_YES, TIMER_CEXBT, "timer_cexbt",    3000, &TimerCexbt_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Cexbt_Callback},
#endif

#ifdef TIMER_ADC
    /****************************************************************************/
    /* 查ADC定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_ADC,   "timer_adc",      1000, &TimerADC_handle,   RT_TIMER_FLAG_PERIODIC, Timeout_ADC_Callback},
#endif

#ifdef TIMER_LED
    /****************************************************************************/
    /* 闪灯定时器                                                              */
    /****************************************************************************/
    {USE_YES, TIMER_LED,   "timer_led",       500, &TimerLED_handle,   RT_TIMER_FLAG_ONE_SHOT, Timeout_LED_Callback},
#endif

#ifdef TIMER_RETCP
    /****************************************************************************/
    /* 重建TCP定时器                                                            */
    /****************************************************************************/
    {USE_YES, TIMER_RETCP, "timer_retcp",   15000, &TimerRetcp_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Retcp_Callback},
#endif

#ifdef TIMER_CHACC
    /****************************************************************************/
    /* 查询ACC定时器                                                            */
    /****************************************************************************/
    {USE_YES, TIMER_CHACC, "timer_chacc",    1000, &TimerChacc_handle, RT_TIMER_FLAG_PERIODIC, Timeout_Check_ACC_Callback},
#endif

#ifdef TIMER_SINGLE
    /****************************************************************************/
    /* 通用单次定时器                                                          */
    /****************************************************************************/
    {USE_YES, TIMER_SINGLE, "timer_single",  1000, &TimerSingle_handle, RT_TIMER_FLAG_ONE_SHOT, Timeout_Single_Callback},
#endif

};

/******************************************************************************/
/* 任务句柄和消息队列句柄                                                    */
/******************************************************************************/
#ifdef TASK_MAIN
    rt_mq_t       main_mq = NULL;
    rt_uint8_t    main_mq_cnt = 0;
    rt_thread_t   main_TaskHandle = NULL;
#endif

#ifdef TASK_APP
    rt_mq_t       app_mq = NULL;
    rt_uint8_t    app_mq_cnt = 0;
    rt_thread_t   app_TaskHandle = NULL;
#endif

#ifdef TASK_CAN
    rt_mq_t       can_mq = NULL;
    rt_uint8_t    can_mq_cnt = 0;
    rt_thread_t   can_TaskHandle = NULL;
#endif

#ifdef TASK_ADC
    rt_mq_t       adc_mq = NULL;
    rt_uint8_t    adc_mq_cnt = 0;
    rt_thread_t   adc_TaskHandle = NULL;
#endif

#ifdef TASK_IMU
    rt_mq_t       imu_mq = NULL;
    rt_uint8_t    imu_mq_cnt = 0;
    rt_thread_t   imu_TaskHandle = NULL;
#endif

#ifdef TASK_FLASH
    rt_mq_t       flash_mq = NULL;
    rt_uint8_t    flash_mq_cnt = 0;
    rt_thread_t   flash_TaskHandle = NULL;
#endif

#ifdef TASK_GPRS
    rt_mq_t       gprs_mq = NULL;
    rt_uint8_t    gprs_mq_cnt = 0;
    rt_thread_t   gprs_TaskHandle = NULL;
#endif

#ifdef TASK_GPS
    rt_mq_t       gps_mq = NULL;
    rt_uint8_t    gps_mq_cnt = 0;
    rt_thread_t   gps_TaskHandle = NULL;
#endif

#ifdef TASK_GPIO
    rt_mq_t       gpio_mq = NULL;
    rt_uint8_t    gpio_mq_cnt = 0;
    rt_thread_t   gpio_TaskHandle = NULL;
#endif

#ifdef TASK_LED
    rt_mq_t       led_mq = NULL;
    rt_uint8_t    led_mq_cnt = 0;
    rt_thread_t   led_TaskHandle = NULL;
#endif

#ifdef TASK_IWDG
    rt_mq_t       iwdg_mq = NULL;
    rt_uint8_t    iwdg_mq_cnt = 0;
    rt_thread_t   iwdg_TaskHandle = NULL;
#endif

#ifdef TASK_EXBT
    rt_mq_t       exbt_mq = NULL;
    rt_uint8_t    exbt_mq_cnt = 0;
    rt_thread_t   exbt_TaskHandle = NULL;
#endif

#ifdef TASK_DTU
    rt_mq_t       dtu_mq = NULL;
    rt_uint8_t    dtu_mq_cnt = 0;
    rt_thread_t   dtu_TaskHandle = NULL;
#endif

/******************************************************************************/
/* 任务信息表(任务列表)                                                      */
/******************************************************************************/
TASK_CLASS gTaskMap[TASK_NUMBER] =
{
#ifdef TASK_MAIN
    /****************************************************************************/
    /* MAIN任务                                                                 */
    /****************************************************************************/
    {
        USE_YES, TASK_MAIN,           MAIN_Task_Handle,   "task_main",   1536,
        NULL, 1, 100,       &main_TaskHandle,     &main_mq,     "main_mq",
        &main_mq_cnt,      20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_APP
    /****************************************************************************/
    /* APP任务                                                                  */
    /****************************************************************************/
    {
        USE_YES, TASK_APP,   APP_Task_Handle,    "task_app",    2048,
        NULL, 2, 100,       &app_TaskHandle,      &app_mq,      "app_mq",
        &app_mq_cnt,       60, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_CAN
    /****************************************************************************/
    /* CAN任务                                                                  */
    /****************************************************************************/
    {
        USE_YES, TASK_CAN,            CAN_Task_Handle,    "task_can",    2048,
        NULL, 5, 100,       &can_TaskHandle,      &can_mq,      "can_mq",
        &can_mq_cnt,       20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_ADC
    /****************************************************************************/
    /* ADC任务                                                                  */
    /****************************************************************************/
    {
        USE_YES, TASK_ADC,            ADC_Task_Handle,    "task_adc",    1024,
        NULL, 5, 100,       &adc_TaskHandle,      &adc_mq,      "adc_mq",
        &adc_mq_cnt,       20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_IMU
    /****************************************************************************/
    /* IMU任务                                                                  */
    /****************************************************************************/
    {
        USE_YES, TASK_IMU,            IMU_Task_Handle,    "task_imu",     1024,
        NULL, 5, 100,       &imu_TaskHandle,      &imu_mq,       "imu_mq",
        &imu_mq_cnt,       20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_FLASH
    /****************************************************************************/
    /* FLASH任务                                                                */
    /****************************************************************************/
    {
        USE_YES, TASK_FLASH,          FLASH_Task_Handle,  "task_flash",    1024,
        NULL, 6, 100,       &flash_TaskHandle,    &flash_mq,      "flash_mq",
        &flash_mq_cnt,     20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_GPRS
    /****************************************************************************/
    /* GPRS任务                                                                 */
    /****************************************************************************/
    {
        USE_YES, TASK_GPRS,           GPRS_Task_Handle,   "task_gprs",     1024,
        NULL, 7, 100,       &gprs_TaskHandle,     &gprs_mq,       "gprs_mq",
        &gprs_mq_cnt,      60, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_GPS
    /****************************************************************************/
    /* GPS任务                                                                 */
    /****************************************************************************/
    {
        USE_YES, TASK_GPS,            GPS_Task_Handle,    "task_gps",      1024,
        NULL, 8, 100,       &gps_TaskHandle,      &gps_mq,        "gps_mq",
        &gps_mq_cnt,       20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_GPIO

    /****************************************************************************/
    /* GPIO任务                                                                 */
    /****************************************************************************/
    {
        USE_YES, TASK_GPIO,           GPIO_Task_Handle,   "task_gpio",     1024,
        NULL, 5, 100,       &gpio_TaskHandle,     &gpio_mq,       "gpio_mq",
        &gpio_mq_cnt,      20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_IWDG
    /****************************************************************************/
    /* IWDG任务                                                                 */
    /****************************************************************************/
    {
        USE_YES, TASK_IWDG,            IWDG_Task_Handle,   "task_iwdg",    1024,
        NULL, 30, 100,       &iwdg_TaskHandle,     &iwdg_mq,      "iwdg_mq",
        &iwdg_mq_cnt,       20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_LED
    /****************************************************************************/
    /* LED任务                                                                  */
    /****************************************************************************/
    {
        USE_YES, TASK_LED,             LED_Task_Handle,    "task_led",     512,
        NULL, 4, 100,        &led_TaskHandle,      &led_mq,       "led_mq",
        &led_mq_cnt,        20, sizeof(MSG_CLASS),    RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_EXBT
    /****************************************************************************/
    /* EXBT任务                                                                 */
    /****************************************************************************/
    {
        USE_YES, TASK_EXBT,            EXBT_Task_Handle,   "task_exbt",    1024,
        NULL, 8, 100,        &exbt_TaskHandle,     &exbt_mq,      "exbt_mq",
        &exbt_mq_cnt,       20, sizeof(MSG_CLASS),    RT_IPC_FLAG_FIFO
    },
#endif

#ifdef TASK_DTU
    /****************************************************************************/
    /* DTU任务                                                                  */
    /****************************************************************************/
    {
        USE_YES, TASK_DTU,             DTU_Task_Handle,    "task_dtu",     1024,
        NULL, 8, 100,        &dtu_TaskHandle,      &dtu_mq,       "dtu_mq",
        &dtu_mq_cnt,        20, sizeof(MSG_CLASS),   RT_IPC_FLAG_FIFO
    },
#endif

};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Timeout_xxx_Callback                                             */
/* 功能描述: 定时器超时回调函数                                              */
/* 输入参数: parameter --- 参数                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-03-18              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
#ifdef TIMER_SENDOK
static void Timeout_Sendok_Callback(void *parameter)
{
    gprs_state.tcp_state.state = TCP_off;
    gprs_state.tcp_state.login = NoLogin;
    set_error_cnt(ERR_CNT_TYPE2);
    dbug("Change tcp state to OFF by timeout!");
    return;
}
#endif

#ifdef TIMER_RETCP
static void Timeout_Retcp_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_RETCP, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_LED
static void Timeout_LED_Callback(void *parameter)
{
    /* 关闪灯 */
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_LED, LED2_FAST_RST, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD21
static void Timeout_Cmd21_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CMD21, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD22
static void Timeout_Cmd22_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CMD22, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD2D
static void Timeout_Cmd2d_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CMD2D, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD51
static void Timeout_Cmd51_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CMD51, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD41
static void Timeout_Cmd41_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_APP, APP_SEND_TCP_DATA, APP_SEND_CMD41, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD42
static void Timeout_Cmd42_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CMD42, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CMD43
static void Timeout_Cmd43_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CMD43, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_SENDL
static void Timeout_SendLink_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_APP, APP_TCP_DATA_LINK, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_ADC
static void Timeout_ADC_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_ADC, TASK_ADC_VAL, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CEXBT
static void Timeout_Cexbt_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_CEXBT, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CANFD
static void Timeout_Check_CanData_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_CAN, CAN_CHECK_DATA, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_CHACC
static void Timeout_Check_ACC_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_CAN, CAN_CHECK_ACC, 0, MSG_UNKNOWN, 0, NULL, 0);
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_CAN, CAN_CHECK_DOOR, 0, MSG_UNKNOWN, 0, NULL, 0);
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_CAN, CAN_CHECK_LIGHT, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_SINGLE
static void Timeout_Single_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_MAIN, SHIFT_TIMER_SINGL, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_INWDT
static void Timeout_Inwdt_Callback(void *parameter)
{
    Fsm_Task_SendAsyncMsg(TASK_TIME, TASK_IWDG, TASK_IWDG_FEED_DOG, 0, MSG_UNKNOWN, 0, NULL, 0);
    return;
}
#endif

#ifdef TIMER_EXWDT
static void Timeout_Exwdt_Callback(void *parameter)
{
    rt_device_control(rt_device_find("exwdt"), RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
    return;
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_AppTaskCreate                                                */
/* 功能描述: 创建任务                                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
static void Fsm_AppTaskCreate(void)
{
    rt_uint32_t  cnts;

    for (cnts = 0; cnts < (sizeof(gTaskMap) / sizeof(TASK_CLASS)); cnts++)
    {
        if (USE_YES == gTaskMap[cnts].taskUse)
        {
            *gTaskMap[cnts].taskHandle = rt_thread_create(gTaskMap[cnts].taskName,
                                         gTaskMap[cnts].taskEntry,
                                         gTaskMap[cnts].taskParam,
                                         gTaskMap[cnts].taskStack,
                                         gTaskMap[cnts].taskPrio,
                                         gTaskMap[cnts].taskTick);

            if (RT_NULL == *gTaskMap[cnts].taskHandle)
            {
                dbug("gTaskMap[%d].taskHandle is NULL!\r\n", cnts);
                Error_Handler();
            }
            else
            {
                rt_thread_startup(*gTaskMap[cnts].taskHandle);
            }

            *gTaskMap[cnts].queueHandle = rt_mq_create(gTaskMap[cnts].queueName,
                                          gTaskMap[cnts].queueItem,
                                          gTaskMap[cnts].queueDeep,
                                          gTaskMap[cnts].queueFlag);

            if (NULL == *gTaskMap[cnts].queueHandle)
            {
                dbug("gTaskMap[%d].taskHandle is NULL!\r\n", cnts);
                Error_Handler();
            }
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_CheckAppTaskConfig                                           */
/* 功能描述: 检查任务表中的配置参数是否满足要求                              */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: uint16_t                                                         */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
static void Fsm_CheckAppTaskConfig(void)
{
    uint16_t cnts;

    for (cnts = 0; cnts < sizeof(gTaskMap) / sizeof(TASK_CLASS); cnts++)
    {
        if ((USE_YES != gTaskMap[cnts].taskUse) && (USE_NO  != gTaskMap[cnts].taskUse))
        {
            Error_Handler();
        }

        if (USE_YES != gTaskMap[cnts].taskUse)
        {
            continue;
        }

        if ((0 == gTaskMap[cnts].taskID) || (0xFF == gTaskMap[cnts].taskID))
        {
            Error_Handler();
        }

        if (NULL == gTaskMap[cnts].taskEntry)
        {
            Error_Handler();
        }

        if (0 == strlen(gTaskMap[cnts].taskName))
        {
            Error_Handler();
        }

        if (128 > gTaskMap[cnts].taskStack)
        {
            Error_Handler();
        }

        if ((RT_THREAD_PRIORITY_MAX - 1) < gTaskMap[cnts].taskPrio)
        {
            Error_Handler();
        }

        if (NULL == gTaskMap[cnts].taskHandle)
        {
            Error_Handler();
        }

        if (0 == gTaskMap[cnts].taskTick)
        {
            Error_Handler();
        }

        if (0 == strlen(gTaskMap[cnts].queueName))
        {
            Error_Handler();
        }

        if (NULL == gTaskMap[cnts].queueHandle)
        {
            Error_Handler();
        }

        if (0 == gTaskMap[cnts].queueDeep)
        {
            Error_Handler();
        }

        if (0 == gTaskMap[cnts].queueItem)
        {
            Error_Handler();
        }

        if (NULL == gTaskMap[cnts].queueCnt)
        {
            Error_Handler();
        }
    }

    return;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_SendAsyncMsg                                                 */
/* 功能描述: 发送异步消息                                                     */
/* 输入参数: ptMsg --- 消息指针                                               */
/*           bUrgent --- 是否为紧急消息(!0紧急)                               */
/* 输出参数: 无                                                               */
/* 返 回 值: USE_STATUS                                                       */
/* 操作流程: 消息的接收者会释放自己的消息队列，而不是发送者的消息队列       */
/* 其它说明: 0:一般消息; !0紧急消息                                          */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
int Fsm_SendAsyncMsg(MSG_CLASS *ptMsg, uint8_t bUrgent)
{
    rt_err_t     ret    = RT_EOK;
    TASK_CLASS  *ptTask = NULL;
    rt_err_t     xRslt  = RT_EOK;

    /**************************************************************************/
    /* 输入参数检查                                                           */
    /**************************************************************************/
    if (NULL == ptMsg)
    {
        ret = _POINTER_IS_NULL;
        goto EXIT_LABEL;
    }

    /**************************************************************************/
    /* 获取消息接收者的任务句柄                                               */
    /**************************************************************************/
    ptTask = Fsm_GetAppTaskInfo(ptMsg->receiver);

    if (NULL == ptTask)
    {
        ret = _POINTER_IS_NULL;
        goto EXIT_LABEL;
    }

    /**************************************************************************/
    /* 发送消息,消息内容会拷贝至mq中全局存在,调用rt_mq_recv释放一个消息实体 */
    /**************************************************************************/
    if (0 == bUrgent)
    {
        xRslt = rt_mq_send(*(ptTask->queueHandle), (void *)ptMsg, sizeof(*ptMsg));
    }
    else
    {
        xRslt = rt_mq_urgent(*(ptTask->queueHandle), (void *)ptMsg, sizeof(*ptMsg));
    }

    if (RT_EOK != xRslt)
    {
        ret = xRslt;
    }
    else
    {
        (*ptTask->queueCnt)++;
    }

EXIT_LABEL:
    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_RecvAsyncMsg                                                 */
/* 功能描述: 接收消息                                                         */
/* 输入参数: ptMsg --- 指向存放消息的指针                                     */
/*           wait --- 接收等待时间，以tick为单位                              */
/* 输出参数: 无                                                               */
/* 返 回 值:                                                                  */
/* 操作流程: 试图接收当前任务的消息队列                                      */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
int Fsm_RecvAsyncMsg(MSG_CLASS *ptMsg, int wait)
{
    rt_err_t     ret    = RT_EOK;
    TASK_CLASS  *ptTask = NULL;
    rt_err_t     xRslt  = RT_EOK;

    /**************************************************************************/
    /* 输入参数检查                                                           */
    /**************************************************************************/
    if (NULL == ptMsg)
    {
        ret = _POINTER_IS_NULL;
        goto EXIT_LABEL;
    }

    /**************************************************************************/
    /* 获取当前任务的消息队列                                                 */
    /**************************************************************************/
    ptTask = Fsm_GetCurrAppTaskInfo();

    if (NULL == ptTask)
    {
        ret = _POINTER_IS_NULL;
        goto EXIT_LABEL;
    }

    xRslt = rt_mq_recv(*(ptTask->queueHandle), ptMsg, sizeof(*ptMsg), wait);

    if (RT_EOK != xRslt)
    {
        ret = xRslt;
        goto EXIT_LABEL;
    }
    else
    {
        (*ptTask->queueCnt)--;
    }

EXIT_LABEL:
    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_GetOperCode                                                  */
/* 功能描述: 提取消息中的消息码作为操作类型                                  */
/* 输入参数: ptMsg --- 指向消息的指针                                         */
/* 输出参数: 无                                                               */
/* 返 回 值: uint16_t                                                         */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
uint16_t Fsm_GetOperCode(void *ptMsg)
{
    uint16_t   opType = 0;
    MSG_CLASS *ptRcvMsg = NULL;

    ptRcvMsg = (MSG_CLASS *)ptMsg;

    if (NULL != ptRcvMsg)
    {
        opType = ptRcvMsg->code;
    }

    return opType;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_FunHandle                                                    */
/* 功能描述: 命令码索引对应执行函数                                          */
/* 输入参数: ptMsg --- 参数结构指针                                           */
/*           *ptCurStat --- 对应的任务执行地址                                */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_FunHandle(MSG_CLASS *ptMsg, FUN_CLASS **ptCurStat)
{
    uint32_t  cnts         = 0;
    uint16_t  nextOperType = 0;
    uint16_t  operType     = _OPER_TYPE_UNKNOWN;

    if ((NULL != ptMsg) && (NULL != ptCurStat) && (NULL != *ptCurStat))
    {
        operType = Fsm_GetOperCode(ptMsg);

        do
        {
            if (operType == (*ptCurStat)[cnts].code)
            {
                break;
            }
            else
            {
                cnts++;
                nextOperType = (*ptCurStat)[cnts].code;
            }
        }
        while (_OPER_TYPE_UNKNOWN != nextOperType);

        if (NULL != (*ptCurStat)[cnts].pfFUNc)
        {
            ((*ptCurStat)[cnts].pfFUNc)(ptMsg);
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_GetAppTaskInfo                                               */
/* 功能描述: 通过任务ID从任务配置表中获取任务的信息                          */
/* 输入参数: usTaskId --- 任务ID                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: TASK_CLASS (任务指针)                                            */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
TASK_CLASS *Fsm_GetAppTaskInfo(uint16_t usTaskId)
{
    uint32_t    ulx;
    TASK_CLASS *ptRet = NULL;

    for (ulx = 0; ulx < (sizeof(gTaskMap) / sizeof(TASK_CLASS)); ulx++)
    {
        if (usTaskId == gTaskMap[ulx].taskID)
        {
            ptRet = &(gTaskMap[ulx]);
            break;
        }
    }

    return ptRet;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_GetCurrAppTaskInfo                                           */
/* 功能描述: 从任务配置表中获取当前正在运行的任务的信息                      */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: TASK_CLASS (任务指针)                                            */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
TASK_CLASS *Fsm_GetCurrAppTaskInfo(void)
{
    uint32_t      ulx;
    TASK_CLASS   *ptRet = NULL;
    rt_thread_t   curTaskHndl = NULL;

    curTaskHndl = rt_thread_self();

    for (ulx = 0; ulx < (sizeof(gTaskMap) / sizeof(TASK_CLASS)); ulx++)
    {
        if (curTaskHndl == *gTaskMap[ulx].taskHandle)
        {
            ptRet = &(gTaskMap[ulx]);
            break;
        }
    }

    return ptRet;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_Task_SendAsyncMsg                                            */
/* 功能描述: 任务间释放消息来通信                                            */
/* 输入参数: 消息结构体                                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-03-17              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fsm_Task_SendAsyncMsg(uint16_t sender, uint16_t receiver, uint16_t code, uint16_t cbcode, uint16_t result, uint16_t len, uint8_t *pbuff, uint8_t urgent)
{
    rt_err_t   res  = RT_EOK;
    MSG_CLASS  tMsg = {0};

    tMsg.receiver = receiver;
    tMsg.sender   = sender;
    tMsg.code     = code;
    tMsg.cbcode   = cbcode;
    tMsg.result   = result;

    if (len > 0)
    {
        tMsg.length = len;
        tMsg.pvBuf = Fsm_GetUB(tMsg.length);
        if (NULL == tMsg.pvBuf)
        {
            dbug("\r\nNo mem for msg!\r\n");
            return;
        }
        else
        {
            memcpy(tMsg.pvBuf, pbuff, len);
        }
    }
    else
    {
        tMsg.length = 0;
        tMsg.pvBuf = NULL;
    }

    res = Fsm_SendAsyncMsg(&tMsg, urgent);

    if (EXE_OK != res)
    {
        Fsm_RetUB(tMsg.pvBuf);
        dbug("Task[%#x] Send Async Msg[%#x] to Task[%#x] is err[%d]!\r\n", sender, code, receiver, res);
        Error_Handler();
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_Task_RecvAsyncMsgHndl                                        */
/* 功能描述: 任务处理消息函数                                                 */
/* 输入参数: ptMsg --- 消息体指针                                             */
/*           *fun_table --- 函数表指针                                        */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-03-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fsm_Task_RecvAsyncMsgHndl(MSG_CLASS *ptMsg, FUN_CLASS **fun_table)
{
    /**************************************************************************/
    /* 执行状态机                                                             */
    /**************************************************************************/
    Fsm_FunHandle(ptMsg, fun_table);

    /**************************************************************************/
    /* 释放消息体                                                             */
    /**************************************************************************/
    if ((NULL != ptMsg) && (NULL != ptMsg->pvBuf))
    {
        Fsm_RetUB(ptMsg->pvBuf);
        ptMsg->pvBuf = NULL;
        ptMsg->length = 0;
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_TimerCreate                                                  */
/* 功能描述: 通用定时器创建                                                  */
/* 输入参数: 定时器ID,0:全创建, !0:指定创建                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_TimerCreate(uint8_t timeID)
{
    uint16_t ulx = 0;

    if (0 == timeID)
    {
        for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
        {
            if (USE_YES == gTimerMap[ulx].timeUse)
            {
                *gTimerMap[ulx].timeHandle = rt_timer_create(gTimerMap[ulx].timeName,
                                             gTimerMap[ulx].timeCallback,
                                             &gTimerMap[ulx].timeID,
                                             gTimerMap[ulx].timePeriod,
                                             gTimerMap[ulx].timeReload);

                if (RT_NULL == *gTimerMap[ulx].timeHandle)
                {
                    dbug("1.gTimerMap[%d].timeHandle is NULL!\r\n", ulx);
                    Error_Handler();
                }
            }
        }
    }
    else
    {
        for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
        {
            if (timeID == gTimerMap[ulx].timeID)
            {
                *gTimerMap[ulx].timeHandle = rt_timer_create(gTimerMap[ulx].timeName,
                                             gTimerMap[ulx].timeCallback,
                                             &gTimerMap[ulx].timeID,
                                             gTimerMap[ulx].timePeriod,
                                             gTimerMap[ulx].timeReload);

                if (RT_NULL == *gTimerMap[ulx].timeHandle)
                {
                    dbug("2.gTimerMap[%d].timeHandle is NULL!\r\n", ulx);
                    Error_Handler();
                }

                break;
            }
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_TimerStart                                                   */
/* 功能描述: 通用定时器启动                                                   */
/* 输入参数: 定时器ID,0:全开启, !0:指定开启                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_TimerStart(uint8_t timeID)
{
    uint16_t ulx = 0;

    if (0 == timeID)
    {
        for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
        {
            if (USE_YES == gTimerMap[ulx].timeUse)
            {
                if (!((*gTimerMap[ulx].timeHandle)->parent.flag & RT_TIMER_FLAG_ACTIVATED))
                {
                    if (RT_EOK != rt_timer_start(*gTimerMap[ulx].timeHandle))
                    {
                        dbug("1.gTimerMap[%d].time start err!\r\n", ulx);
                        Error_Handler();
                    }
                }
            }
        }
    }
    else
    {
        for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
        {
            if (timeID == gTimerMap[ulx].timeID)
            {
                if (!((*gTimerMap[ulx].timeHandle)->parent.flag & RT_TIMER_FLAG_ACTIVATED))
                {
                    if (RT_EOK != rt_timer_start(*gTimerMap[ulx].timeHandle))
                    {
                        dbug("2.gTimerMap[%d].time start err!\r\n", ulx);
                        Error_Handler();
                    }
                    break;
                }
            }
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_TimerStop                                                    */
/* 功能描述: 通用定时器停止                                                   */
/* 输入参数: 定时器ID,0:全关闭, !0:指定关闭                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_TimerStop(uint8_t timeID)
{
    uint32_t ulx;

    if (0 == timeID)
    {
        for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
        {
            if ((*gTimerMap[ulx].timeHandle)->parent.flag & RT_TIMER_FLAG_ACTIVATED)
            {
                if (RT_EOK != rt_timer_stop(*gTimerMap[ulx].timeHandle))
                {
                    dbug("1.gTimerMap[%d].time stop err!\r\n", ulx);
                    Error_Handler();
                }
            }
        }
    }
    else
    {
        for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
        {
            if (timeID == gTimerMap[ulx].timeID)
            {
                if ((*gTimerMap[ulx].timeHandle)->parent.flag & RT_TIMER_FLAG_ACTIVATED)
                {
                    if (RT_EOK != rt_timer_stop(*gTimerMap[ulx].timeHandle))
                    {
                        dbug("2.gTimerMap[%d].time stop err!\r\n", timeID);
                        Error_Handler();
                    }
                    break;
                }
            }
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_TimerControl                                                 */
/* 功能描述: 通用定时器控制(变更周期)                                        */
/* 输入参数: timeID --- 定时器ID                                              */
/*           cmd ： RT_TIMER_CTRL_GET_TIME ：定时器当前时间获取               */
/*                  RT_TIMER_CTRL_SET_TIME：定时时间设置                      */
/*                  RT_TIMER_CTRL_SET_ONESHOT：设置为单次定时                 */
/*                  RT_TIMER_CTRL_SET_PERIODIC：设置为周期定时器              */
/*           arg --- 定时器时间设置/读取                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_TimerControl(uint8_t timeID, uint8_t cmd, void *arg)
{
    uint32_t ulx;

    for (ulx = 0; ulx < (sizeof(gTimerMap) / sizeof(TIME_CLASS)); ulx++)
    {
        if (timeID == gTimerMap[ulx].timeID)
        {
            if (NULL == gTimerMap[ulx].timeHandle)
            {
                Error_Handler();
            }

            if (RT_EOK != rt_timer_control(*gTimerMap[ulx].timeHandle, cmd, arg))
            {
                dbug("gTimerMap[%d].time control err!\r\n", ulx);
                Error_Handler();
            }

            break;
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_SysReset                                                     */
/* 功能描述: 系统复位                                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_SysReset(void)
{
    __set_FAULTMASK(1);  /* 关闭所有中断 */
    NVIC_SystemReset();  /* 复位 */
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_GetUB                                                        */
/* 功能描述: 申请内存函数                                                     */
/* 输入参数: size --- 申请内存的大小，以字节为单位                            */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void *Fsm_GetUB(uint32_t size)
{
    return malloc(size);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_RetUB                                                        */
/* 功能描述: 释放内存函数                                                     */
/* 输入参数: pBuf --- 指向需要释放的内存的指针                                */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-01-21              v 1.0.0       Leonard         创建函数         */
/*<FUNC->**********************************************************************/
void Fsm_RetUB(void *pBuf)
{
    free(pBuf);
    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fsm_Appinit_Task                                                 */
/* 功能描述: 初始化任务                                                       */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 初始化完毕后删除此任务                                          */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-03-16              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void Fsm_Appinit_Task(void *parameter)
{
    rt_base_t  level;

    /* 关闭中断 */
    level = rt_hw_interrupt_disable();
    /* 创建定时器 */
    Fsm_TimerCreate(0);
    /* 检查任务参数 */
    Fsm_CheckAppTaskConfig();
    /* 创建任务 */
    Fsm_AppTaskCreate();
    /* 开启中断 */
    rt_hw_interrupt_enable(level);

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Application_Init                                                 */
/* 功能描述: 任务创建前的相关初始化                                          */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-03-16              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Application_Init(void)
{
    rt_thread_t  init_thread = RT_NULL;

    init_thread = rt_thread_create("fsm_init", Fsm_Appinit_Task, RT_NULL, 1024, 16, 20);

    if (RT_NULL == init_thread)
    {
        dbug("\r\nstart fsm create fail!\r\n");
    }
    else
    {
        rt_thread_startup(init_thread);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: standard_to_stamp                                                */
/* 功能描述: 标准时间准换成时间戳                                            */
/* 输入参数: str_time --- 标准时间格式字符串 例如："2012-12-12 12:12:12"     */
/* 输出参数: UTC时间戳                              "2012/12/12,12:12:12"     */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 获取0经度时间,网页显示的自动加了时区                            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-06              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int standard_to_stamp(char *str_time)
{
    char *pt;
    int  utc;
    struct tm stm = {0};
    int iY, iM, iD, iH, iMin, iS;

    pt = str_time;
    iY = atoi(pt);

    pt = (char *)strstr(pt, "/") + 1;
    iM = atoi(pt);

    pt = (char *)strstr(pt, "/") + 1;
    iD = atoi(pt);

    pt = (char *)strstr(pt, ",") + 1;
    iH = atoi(pt);

    pt = (char *)strstr(pt, ":") + 1;
    iMin = atoi(pt);

    pt = (char *)strstr(pt, ":") + 1;
    iS = atoi(pt);

    stm.tm_year = iY - 1900;
    stm.tm_mon  = iM - 1;
    stm.tm_mday = iD;
    stm.tm_hour = iH;
    stm.tm_min  = iMin;
    stm.tm_sec  = iS;

    utc = (int)mktime(&stm);
//  utc = utc - TIME_ZOOM;
//  dbug("\r\n%d-%0d-%0d %0d:%0d:%0d -> [0x%.x]\r\n", iY, iM, iD, iH, iMin, iS, utc);

    return utc;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: stamp_to_standard                                                */
/* 功能描述: 时间戳转换为标准时间格式                                        */
/* 输入参数: stampTime --- 时间戳                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: Times                                                            */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-06              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
Times stamp_to_standard(int stampTime)
{
    char *pt = NULL;
    char s[32] = {0};
    struct tm tm;
    Times standard;
    unsigned int tick = stampTime + TIME_ZOOM;

    tm = *localtime(&tick);  // 获取系统标准时间
//strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);  //格式1
    strftime(s, sizeof(s), "%Y/%m/%d,%H:%M:%S", &tm);  //格式2
    dbug("\r\n[0x%.x] -> %s\r\n", (int)tick - TIME_ZOOM, s);

    pt = s;
    standard.Year = atoi(s);

    pt = (char *)strstr(pt, "/") + 1;
    standard.Mon = atoi(pt);

    pt = (char *)strstr(pt, "/") + 1;
    standard.Day = atoi(pt);

    pt = (char *)strstr(pt, ",") + 1;
    standard.Hour = atoi(pt);

    pt = (char *)strstr(pt, ":") + 1;
    standard.Min = atoi(pt);

    pt = (char *)strstr(pt, ":") + 1;
    standard.Second = atoi(pt);

    return standard;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Link_add_node_to_tail                                            */
/* 功能描述: 新建一个节点并添加到链表末尾，返回添加节点后的链表的头指针     */
/* 输入参数: link_head --- 链表的头指针                                      */
/*           node_data --- 数据域指针                                         */
/*           type --- 实时/历史数据                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: struct link                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 节点序号从1开始计数                                             */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
struct link *Link_add_node_to_tail(struct link *link_head, void *node_data, char type)
{
    struct link *pt = NULL;
    struct link *node = NULL;

    if (NULL == link_head)
    {
        return NULL;
    }

    /* 创建新节点 */
    node = (struct link *)malloc(sizeof(struct link));
    if (node == NULL)
    {
        dbug("\r\nNo mem for link add!\r\n");
        return NULL;
    }

    /* 空链表 */
    if (link_head->next == NULL)
    {
        /* 将新节点置为头节点 */
        link_head->next = node;
    }
    else  /* 非空链表则将新建节点添加到表尾 */
    {
        pt = link_head;
        /* 若未到表尾，则移动节点直到表尾 */
        while (pt->next != NULL)
        {
            /* 移动节点 */
            pt = pt->next;
        }

        /* 让末节点指向新建的节点 */
        pt->next = node;
    }

    /* 链表节点总数加1 */
    link_head->sernum++;
    /* 新节点序号 */
    node->sernum = link_head->sernum;
    /* 新节点数据域赋值 */
    node->data = node_data;
    /* 新节点设置为表尾 */
    node->next = NULL;
    /* 打印提示 */
    dbug("\r\nAdd a %s node[%#x], link cunt = [%d]!\r\n", ((type == 0) ? "new" : "old"), ((pack_item_t)node_data)->serial, link_head->sernum);
    /* 返回添加节点后的链表头指针 */
    return link_head;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Link_delete_all_node                                             */
/* 功能描述: 删除链表所有节点                                                */
/* 输入参数: link_head --- 链表的头指针                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: struct link                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
struct link *Link_delete_all_node(struct link *link_head)
{
    rt_uint32_t  index;
    struct link *node = NULL;
    struct pack_item *pack = NULL;
    rt_uint32_t  num = link_head->sernum;

    for (index = 0; index < num; index++)
    {
        node = link_head->next;
        link_head->next = node->next;
        pack = (struct pack_item *)node->data;
        free(pack->pbuf);
        /* 释放节点中指向的数据 */
        free(node->data);
        /* 释放已删除节点的内存 */
        free(node);
    }

    link_head->sernum = 0;
    link_head->data = NULL;
    link_head->next = NULL;

    dbug("\r\nLink delete all node as active!\r\n");
    /* 返回空节点的链表头指针 */
    return link_head;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Link_delete_node_either                                          */
/* 功能描述: 从链表中删除任一节点                                             */
/* 输入参数: link_head --- 链表的头指针                                       */
/*           node_num --- 待删除的节点序号                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: struct link                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
struct link *Link_delete_node_either(struct link *link_head, int node_num)
{
    rt_uint32_t  index;
    struct link *last_node = NULL;
    struct link *next_node = NULL;
    struct pack_item *pack = NULL;
    rt_uint32_t  num = link_head->sernum;

    if ((link_head == NULL) || (link_head->next == NULL))
    {
        dbug("\r\nLink is empty!\r\n");
        return NULL;
    }

//  if(node_num > num)
//  {
//    dbug("\r\nNode ser num illegal!\r\n");
//    return NULL;
//  }

    /* 从头节点开始遍历 */
    next_node = link_head->next;
    while (num)
    {
        pack = (struct pack_item *)next_node->data;
        if (node_num == pack->serial)
        {
            break;
        }

        /* 移动节点 */
        num--;
        last_node = next_node;
        next_node = next_node->next;
    }

    if (num > 0)
    {
        /* 如果待删除为头节点 */
        if (next_node == link_head->next)
        {
            link_head->next = next_node->next;
        }
        else
        {
            last_node->next = next_node->next;
        }

        /* 链表节点总数减1 */
        link_head->sernum--;
        free(pack->pbuf);
        /* 释放节点中指向的数据 */
        free(next_node->data);
        /* 释放已删除节点的内存 */
        free(next_node);
        /* 重置各节点序号 */
        next_node = link_head->next;
        for (index = 1; index <= link_head->sernum; index++)
        {
            next_node->sernum = index;
            next_node = next_node->next;
        }
    }
    else
    {
        dbug("\r\nCan't find this node[%#x]!\r\n", node_num);
        return NULL;
    }

    /* 打印提示 */
    dbug("\r\nDelete a node[%#x], link cunt = [%d]!\r\n", node_num, link_head->sernum);
    /* 返回删除节点后的链表头指针 */
    return link_head;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Link_insert_node_either                                          */
/* 功能描述: 在单向链表的任意位置处插入一个节点                              */
/* 输入参数: link_head --- 链表的头指针                                      */
/*           node_data --- 节点数据域数据指针                                */
/*           insert_pos --- 节点插入位置(原链表某个节点序号)                 */
/*           insert_dir --- 节点插入方向(插入位置前(小序号)/后(大序号))      */
/*                          insert_dir=0 前插入; insert_dir!0 后插入          */
/* 输出参数: 无                                                               */
/* 返 回 值: struct link                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 操作前的链表节点序号已按升序排列                                */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
struct link *Link_insert_node_either(struct link *link_head, void *node_data, rt_uint32_t insert_pos, rt_uint8_t insert_dir)
{
    rt_uint32_t  index;
    struct link *new_node = NULL;
    struct link *last_node = NULL;
    struct link *next_node = NULL;
    rt_uint32_t  num = link_head->sernum;

    if ((link_head == NULL) || (link_head->next == NULL))
    {
        dbug("\r\nLink is empty!\r\n");
        return NULL;
    }

//  if(insert_pos > num)
//  {
//    dbug("\r\nNode ser num illegal!\r\n");
//    return NULL;
//  }

    /* 新建一个待插入节点 */
    new_node = (struct link *)malloc(sizeof(struct link));
    if (new_node == NULL)
    {
        dbug("\r\nNo mem for link insert!\r\n");
        return NULL;
    }
    /* 待插入节点指针域赋值为空指针 */
    new_node->next = NULL;
    new_node->data = node_data;

    /* 若原链表为空 */
    if (link_head->next == NULL)
    {
        /* 插入该节点作为头结点 */
        link_head->next = new_node;
    }
    else
    {
        /* 从头节点开始遍历 */
        next_node = link_head->next;
        while (num)
        {
            if (insert_pos == next_node->sernum)
            {
                break;
            }

            /* 移动节点 */
            num--;
            last_node = next_node;
            next_node = next_node->next;
        }

        if (num > 0)
        {
            /* 节点前插入 */
            if (insert_dir == 0)
            {
                /* 如果插入位置为头节点前 */
                if (next_node == link_head->next)
                {
                    /* 新节点指针域指向原链表头结点 */
                    new_node->next = link_head->next;
                    /* 头指针指向新头节点 */
                    link_head->next = new_node;
                }
                else
                {
                    /* 让前一节点指针域指向新节点 */
                    last_node->next = new_node;
                    /* 新节点指针域指向下一节点 */
                    new_node->next = next_node;
                }
            }
            else  /* 节点后插入 */
            {
                /* 如果插入位置为尾节点后 */
                if (NULL == next_node->next)
                {
                    /* 新节点作为尾节点 */
                    next_node->next = new_node;
                }
                else
                {
                    /* 新节点指针域指向下一节点 */
                    new_node->next = next_node->next;
                    /* 让前一节点指针域指向新节点 */
                    next_node->next = new_node;
                }
            }
        }
        else
        {
            dbug("\r\nCan not find this node!\r\n");
            free(new_node);
            return NULL;
        }
    }

    /* 链表节点总数加1 */
    link_head->sernum++;
    /* 重置各节点序号 */
    next_node = link_head->next;
    for (index = 1; index <= link_head->sernum; index++)
    {
        next_node->sernum = index;
        next_node = next_node->next;
    }

    /* 打印提示 */
    dbug("\r\nInsert new node, link cunt=[%d]!\r\n", link_head->sernum);
    /* 返回插入节点后的链表头指针 */
    return link_head;
}


