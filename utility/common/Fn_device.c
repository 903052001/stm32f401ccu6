/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2021. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_device.c                                                      */
/* 内容摘要: 自定义设备驱动框架+软件定时器实现源文件                         */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2021-01-07                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2021-01-07        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "Fn_device.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define MAX_LINE_LENGTH_BYTES          (64)                 /* 单行最大长度 */
#define DEFAULT_LINE_LENGTH_BYTES      (16)                 /* 默认单行长度 */

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/


/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static  T_LIST_ROOT  g_tDeviceList = {NULL};                   /* 设备链表头 */
static  T_LIST_ROOT  g_tTimerList  = {NULL};         /* 软件定时器设备链表头 */

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*
 * Components Initialization will initialize some driver and components as following order:
 * init_start        --> 0
 * PRE_EXPORT        --> 1
 * BOARD_EXPORT      --> 2
 * DEVICE_EXPORT     --> 3
 * COMPONENT_EXPORT  --> 4
 * ENV_EXPORT        --> 5
 * APP_EXPORT        --> 6
 * init_end          --> 6.end
 *
 * These automatically initialization, the driver or component initial function must be defined with:
 * INIT_BOARD_EXPORT(fn);
 * INIT_DEVICE_EXPORT(fn);
 * ...
 * INIT_APP_EXPORT(fn);
 * etc.
 */
static int init_start(void)
{
    return 0;
}
INIT_EXPORT(init_start, "0");

static int init_end(void)
{
    return 0;
}
INIT_EXPORT(init_end, "6.end");

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_hw_us_delay                                                   */
/* 功能描述: 微秒级精准延时                                                   */
/* 输入参数: nus --- 微秒延时数                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 默认Systick时钟为系统时钟即未分频的主频(84MHz)                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-07-28              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_hw_us_delay(int nus)
{
    unsigned int ticks, _1us;
    unsigned int told, tnow, tcnt = 0;
    unsigned int reload = SysTick->LOAD + 1;

    _1us  = reload / (1000 * HAL_TICK_FREQ_DEFAULT);
    ticks = _1us * nus;
    told  = SysTick->VAL;

    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            tcnt += (tnow < told) ? (told - tnow) : (reload - tnow + told);

            told = tnow;

            if (tcnt >= ticks)
            {
                break;
            }
        }
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_auto_init                                              */
/* 功能描述: 自动初始化组件                                                   */
/* 输入参数: 各外设驱动或外设芯片的初始化函数                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程: 同一级的无法保证初始化顺序,所以同一级间的需要独立初始化,不可   */
/* 其它说明: 互相调用;不同级间的按优先级顺序逐级初始化                      */
/* 修改记录: 同一级的初始化顺序由工程中源文件的上下位置决定                 */
/* -------------------------------------------------------------------------- */
/*     2021-01-25              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_auto_init(void)
{
    const init_fun *fn_ptr;

    for (fn_ptr = &__auto_init_init_start; fn_ptr < &__auto_init_init_end; fn_ptr++)
    {
        (*fn_ptr)();
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_list_init                                              */
/* 功能描述: 设备链表初始化                                                   */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_list_init(void)
{
    LIST_INIT_ROOT(g_tDeviceList);
    return 0;
}
INIT_PRE_EXPORT(Fn_device_list_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_find                                                   */
/* 功能描述: 设备查找函数                                                     */
/* 输入参数: name --- 设备名称                                                */
/* 输出参数: 设备句柄                                                         */
/* 返 回 值: struct device *                                                  */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
struct device *Fn_device_find(const char *name)
{
    T_LIST_NODE *node = NULL;

    if ((NULL == name) || (sizeof(name) > DEVICE_NAME_MAXLEN))
    {
        return NULL;
    }

    node = LIST_NEXT_IN(g_tDeviceList);

    while (NULL != node)
    {
        if (0 != strncmp(name, ((struct device *)node)->name, DEVICE_NAME_MAXLEN))
        {
            node = LIST_NEXT_IN(((struct device *)node)->node);
        }
        else
        {
            return (struct device *)node;
        }
    }

    return NULL;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_register                                               */
/* 功能描述: 设备注册函数                                                     */
/* 输入参数: dev --- 设备句柄                                                 */
/*           name --- 设备名称                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_register(struct device *dev, const char *name)
{
    if ((NULL == dev) || (NULL == name))
    {
        return -1;
    }

    if (NULL != Fn_device_find(name))
    {
        return -2;
    }

    LIST_INIT_NOTE(dev->node, dev);
    LIST_INSERT_AFTER(g_tDeviceList, dev->node);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_unregister                                             */
/* 功能描述: 设备注销                                                         */
/* 输入参数: dev --- 设备句柄                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_unregister(struct device *dev)
{
    if (NULL == dev)
    {
        return -1;
    }

    LIST_REMOVE(dev->node);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_init                                                   */
/* 功能描述: 设备初始化函数                                                   */
/* 输入参数: dev --- 设备句柄                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_init(struct device *dev)
{
    return  dev->init(dev);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_open                                                   */
/* 功能描述: 设备开启函数                                                     */
/* 输入参数: dev --- 设备句柄                                                 */
/*           flag --- 打开方式                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_open(struct device *dev, int flag)
{
    int res;

    if (NULL != dev->init)
    {
        res = Fn_device_init(dev);
        if (0 != res)  return -1;
    }

    return  dev->open(dev, flag);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_close                                                  */
/* 功能描述: 设备关闭函数                                                     */
/* 输入参数: dev --- 设备句柄                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_close(struct device *dev)
{
    return  dev->close(dev);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_read                                                   */
/* 功能描述: 设备读入函数                                                     */
/* 输入参数: dev --- 设备句柄                                                 */
/*           pos --- 读取偏移/外设数据源地址                                  */
/*           buffer --- 读入后内存缓存区头                                    */
/*           size --- 欲读取长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int 实际读取到的长度                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_read(struct device *dev, int pos, void *buffer, int size)
{
    return dev->read(dev, pos, buffer, size); 
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_write                                                  */
/* 功能描述: 设备写出函数                                                     */
/* 输入参数: dev --- 设备句柄                                                 */
/*           pos --- 写出偏移/外设数据写入地址                                */
/*           buffer --- 内存中待写出数据区                                    */
/*           size --- 欲写出长度                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int 实际写出的长度                                               */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_write(struct device *dev, int pos, const void *buffer, int size)
{
    return dev->write(dev, pos, buffer, size); 
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_control                                                */
/* 功能描述: 设备控制函数                                                     */
/* 输入参数: dev --- 设备句柄                                                 */
/*           cmd --- 控制命令                                                 */
/*           arg --- 控制参数                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_control(struct device *dev, int cmd, void *arg)
{
    return  dev->control(dev, cmd, arg);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_xfer                                                   */
/* 功能描述: 设备读写统一函数                                                 */
/* 输入参数: dev --- 设备句柄                                                 */
/*           msgs --- 数据结构体指针                                          */
/*           arg --- 数据结构体个数                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_xfer(struct device *dev, void *msgs, int num)
{
    return  dev->xfer(dev, msgs, num);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_set_rx_indicate                                        */
/* 功能描述: 设备接收回调函数绑定                                             */
/* 输入参数: dev --- 设备句柄                                                 */
/*           rxind --- 设备接收回调函数指针                                   */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_device_set_rx_indicate(struct device *dev, rx_indi rxind)
{
    dev->rxind = rxind;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_set_tx_complete                                        */
/* 功能描述: 设备发送完成回调函数绑定                                        */
/* 输入参数: dev --- 设备句柄                                                 */
/*           txdone --- 设备发送完成回调函数指针                              */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_device_set_tx_complete(struct device *dev, tx_comp txdone)
{
    dev->txind = txdone;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_list_init                                               */
/* 功能描述: 软件定时器设备链表初始化                                        */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: Systick作为软件定时器的时基(1ms)                                */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_list_init(void)
{
    LIST_INIT_ROOT(g_tTimerList);
    return 0;
}
INIT_PRE_EXPORT(Fn_timer_list_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_find                                                    */
/* 功能描述: 定时器设备查找函数                                              */
/* 输入参数: index --- 定时器索引号                                           */
/* 输出参数: 设备句柄                                                         */
/* 返 回 值: struct timer *                                                   */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
struct timer *Fn_timer_find(unsigned char index)
{
    T_LIST_NODE *node = NULL;

    if (0 == index) 
    {
        return NULL;
    }
    
    node = LIST_NEXT_IN(g_tTimerList);

    while (NULL != node)
    {
        if (index != ((struct timer *)node)->index)
        {
            node = LIST_NEXT_IN(((struct timer *)node)->node);
        }
        else
        {
            return (struct timer *)node;
        }
    }

    return NULL;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_register                                                */
/* 功能描述: 定时器设备注册函数                                              */
/* 输入参数: index --- 定时器设备索引号                                      */
/*           reload --- 定时器设备超时时间(单位ms)                           */
/*           mode --- 工作模式                                                */
/*           cd --- 定时器超时回调函数                                        */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_register(unsigned char index, unsigned int reload, int mode, int (*cb)(void *))
{
    struct timer *tim = NULL;
    
    if ((0 == index) || (0 == reload) || (NULL == cb))
    {
        return -1;
    }

    if (NULL != Fn_timer_find(index))
    {
        return -2;
    }

#ifdef  BSP_USE_RT_THREAD
    tim = (struct timer *)rt_malloc(sizeof(struct timer));
#else
    tim = (struct timer *)malloc(sizeof(struct timer));
#endif
    
    if (NULL == tim)
    {
        return -3;
    }
    
    memset(tim, 0, sizeof(struct timer));

    tim->mode   = mode;
    tim->index  = index;
    tim->reload = reload;
    tim->tmocb  = cb;
    
    LIST_INIT_NOTE(tim->node, tim);
    LIST_INSERT_AFTER(g_tTimerList, tim->node);
    
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_unregister                                              */
/* 功能描述: 定时器设备注销                                                  */
/* 输入参数: index --- 定时器设备索引号                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_unregister(unsigned char index)
{
    struct timer *tim = NULL;

    tim = Fn_timer_find(index);
      
    if ((0 == index) || (NULL == tim))
    {
        return -1;
    }

    LIST_REMOVE(tim->node);

#ifdef  BSP_USE_RT_THREAD
    rt_free(tim);
#else
    free(tim);
#endif    

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_start                                                   */
/* 功能描述: 定时器设备开启                                                  */
/* 输入参数: index --- 定时器设备索引号                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_start(unsigned char index)
{
    struct timer *tim = NULL;
    
    tim = Fn_timer_find(index);
      
    if ((0 == index) || (NULL == tim))
    {
        return -1;
    }

    tim->current = tim->reload;
    tim->isuse = TIMER_ENABLE;
    
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_close                                                   */
/* 功能描述: 定时器设备关闭                                                  */
/* 输入参数: index --- 定时器设备索引号                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_close(unsigned char index)
{
    struct timer *tim = NULL;
    
    tim = Fn_timer_find(index);
      
    if ((0 == index) || (NULL == tim))
    {
        return -1;
    }

    tim->isuse = TIMER_DISABLE;

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_close                                                   */
/* 功能描述: 定时器设备关闭                                                  */
/* 输入参数: index --- 定时器设备索引号                                      */
/*           cmd --- 控制命令                                                 */
/*           args --- 控制参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_control(unsigned char index, int cmd, void *args)
{
    struct timer *tim = NULL;
    
    tim = Fn_timer_find(index);
      
    if ((0 == index) || (NULL == tim))
    {
        return -1;
    }

    switch (cmd)
    {
    case TIMER_NEW_CYCLE:
        tim->current = 0;
        tim->reload = *(int *)args;
        break;

    case TIMER_NEW_MODE:
        tim->mode = *(int *)args & 0x01;
        break;

    case TIMER_NEW_STATE:
        tim->isuse = *(int *)args & 0x01;
        break;
    
    default:
        break;
    }
    
    return 0;    
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_timer_handle                                                  */
/* 功能描述: 定时器设备超时处理函数                                          */
/* 输入参数: list --- 定时器设备链表                                         */
/* 输出参数: 设备句柄                                                         */
/* 返 回 值: struct timer                                                     */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-11              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_timer_handle(void)
{
    struct timer *tim = NULL; 
    T_LIST_NODE *node = NULL;

    node = LIST_NEXT_IN(g_tTimerList);

    while (NULL != node)
    {
        tim = (struct timer *)node;
        
        if (TIMER_ENABLE == tim->isuse)
        {
            if (tim->current != 0)
            {
                tim->current--;
            }
            else
            {
                if (tim->mode == TIMER_CIRCLE)
                {
                    tim->current = tim->reload;
                }
                else
                {
                    tim->isuse = TIMER_DISABLE;
                }
                
                if (NULL != tim->tmocb)
                {
                    tim->tmocb((void *)tim);
                }
            }
        }

        node = LIST_NEXT_IN(((struct timer *)node)->node);
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_device_version                                                */
/* 功能描述: 版本信息打印                                                     */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_device_version(void)
{
    dbug("\r\n \\ | /\r\n");
    dbug("- IAP -    Version %s Build %s %s\r\n", DEVICE_VERSION, __TIME__, __DATE__);
    dbug(" / | \\     2018 - 2021 Copyright by Leonard private\r\n\r\n");
    return 0;
}
INIT_COMPONENT_EXPORT(Fn_device_version);

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_printbuffer                                                   */
/* 功能描述: 打印缓冲区的内容(hex dump)                                      */
/* 输入参数: addr --- 地址                                                    */
/*           data --- 指向打印数据的指针                                      */
/*           width --- 数据宽度(4/2/1B)                                       */
/*           count --- 数据计数                                               */
/*           linelen --- 行长度                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_printbuffer(unsigned long addr, char *data, unsigned int width, unsigned int count, unsigned int linelen)
{
    int i;
    union linebuf
    {
        unsigned int   ui[MAX_LINE_LENGTH_BYTES / sizeof(unsigned int)   + 1];
        unsigned short us[MAX_LINE_LENGTH_BYTES / sizeof(unsigned short) + 1];
        unsigned char  uc[MAX_LINE_LENGTH_BYTES / sizeof(unsigned char)  + 1];
    } lb;

    if (linelen * width > MAX_LINE_LENGTH_BYTES)
    {
        linelen = MAX_LINE_LENGTH_BYTES / width;
    }

    if (linelen < 1)
    {
        linelen = DEFAULT_LINE_LENGTH_BYTES / width;
    }

    while (count)
    {
        dbug("%08lx:", addr);

        if (count < linelen)
        {
            linelen = count;
        }

        for (i = 0; i < linelen; i++)
        {
            unsigned int x;

            if (width == 4)
            {
                x = lb.ui[i] = *(volatile unsigned int *)data;
            }
            else if (width == 2)
            {
                x = lb.us[i] = *(volatile unsigned short *)data;
            }
            else
            {
                x = lb.uc[i] = *(volatile unsigned char *)data;
            }

            dbug(" %0*x", width * 2, x);

            data += width;
        }

        /* Print data in ASCII characters */
        for (i = 0; i < linelen * width; i++)
        {
            if ((lb.uc[i] < ' ') || (lb.uc[i] > '~'))
            {
                lb.uc[i] = '.';
            }
        }

        lb.uc[i] = '\0';
        dbug("    %s\r\n", lb.uc);

        /* update references */
        addr  += linelen * width;
        count -= linelen;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_hexdump                                                       */
/* 功能描述: 对指定的内存块进行16进制的格式化打印                            */
/* 输入参数: p --- 指定内存块的首地址                                        */
/*           size --- 内存块大小                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程: (void *)0x08000000 / (void *)0x20000000                          */
/* 其它说明: 支持从0x08000000/0x20000000地址处开始的数据打印.                */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_hexdump(const void *p, unsigned int size)
{
    const unsigned char *c = p;
    unsigned int i;

    dbug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
         "~~~~~~~~~~~~~~~~\r\n");
    dbug("ADDRESS    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  "
         "ASCII\r\n");
    dbug("-----------------------------------------------------------"
         "----------------\r\n");

    while (size > 0)
    {
        dbug("%08ph: ", c);

        for (i = 0; i < 16; i++)
        {
            if (i < size)
            {
                dbug("%02x ", c[i]);
            }
            else
            {
                dbug("   ");
            }
        }

        for (i = 0; i < 16; i++)
        {
            if (i < size)
            {
                dbug("%c", (c[i] >= 32 && c[i] < 127) ? c[i] : '.');
            }
            else
            {
                dbug(" ");
            }
        }

        dbug("\r\n");

        c += 16;

        if (size <= 16)
        {
            break;
        }

        size -= 16;
    }

    dbug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
         "~~~~~~~~~~~~~~~~\r\n");
    
    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_print_echo_hex                                                */
/* 功能描述: HEX格式打印回显数据                                              */
/* 输入参数: name --- 标识名                                                  */
/*           buf --- 数据头                                                   */
/*           size --- 数据长                                                  */
/*           width --- 打印宽度                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-05-02              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_print_echo_hex(const char *name, int width, const char *buf, int size)
{
    int i, j;

    dbug("\r\n                ");
    for (i = 0; i < width; i++)
    {
        dbug("%2d ", i + 1);
    }
    dbug("\r\n");

    for (i = 0; i < size; i += width)
    {
        dbug("%s: hex = ", name);
        for (j = 0; j < width; j++)
        {
            if (i + j < size)
            {
                dbug("%02X ", buf[i + j]);
            }
            else
            {
                dbug("   ");
            }
        }
        dbug("\r\n");
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_print_Progressbar                                             */
/* 功能描述: 进度条打印                                                       */
/* 输入参数: index --- 标识名称                                               */
/*           currentLen --- 当前已接收长度                                    */
/*           totalLen --- 数据总长度                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 步进5%,每5%打印一个'#'                                          */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-02-02              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_print_Progressbar(const char *index, unsigned int currentLen, unsigned int totalLen)
{
    int  i, rate = 0;
    char bar[23] = {"[                    ]\0"};
    static int len = 0;
    
    rate = (100 * currentLen / totalLen) % (100 + 1);  /* [0..100] */

    if (0 != len)
    {
        do
        {
            dbug("\b");
        }while(--len);
    }
    else
    {
        dbug("\r\n%s progress : ", index);
    }

    len += dbug("%3d% | ", rate);
    
    for (i = 0; i < rate / 5; i++)
    {
        bar[1 + i] = '#';
    }
    
    len += dbug("%s", bar);
    
    if (rate == 100)
    {
        len = 0;
        dbug("\r\n");
    }

    return;    
}


