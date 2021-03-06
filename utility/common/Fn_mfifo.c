/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.              */
/*                                                                            */
/* 文件名称: Fn_mfifo.c                                                       */
/* 内容摘要: FIFO循环队列实现源文件                                          */
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
#include <stdlib.h>
#include "Fn_mfifo.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/


/******************************************************************************/
/*                              内部数据类型定义                             */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/


/******************************************************************************/
/*                               全局(静态)变量                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/


/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: mfifo_init                                                       */
/* 功能描述: FIFO初始化                                                       */
/* 输入参数: fifo --- 指向FIFO结构体的指针                                    */
/*           buf  --- 指向FIFO缓冲区的地址                                    */
/*           size --- FIFO缓冲区的大小                                        */
/* 输出参数: 无                                                               */
/* 返 回 值: 无                                                               */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int mfifo_init(struct _mfifo *fifo, unsigned char *buf, int size)
{
    fifo->buffer = buf;
    fifo->size   = size;
    fifo->in     = 0;
    fifo->out    = 0;
    fifo->full   = 0;

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: mfifo_put                                                        */
/* 功能描述: 向FIFO缓冲区写入数据                                             */
/* 输入参数: fifo --- 指向FIFO结构体的指针                                    */
/*           buf  --- 指向写入数据的指针                                      */
/*           len  --- 写入数据长度                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: 写入数据的长度                                                   */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int mfifo_put(struct _mfifo *fifo, unsigned char *buf, int len)
{
    int i = 0;
    
    for (i = 0; i < len; i++)
    {
        fifo->buffer[fifo->in++] = buf[i];

        if (fifo->in >= fifo->size)
        {
            fifo->in = 0;
        }

        if (fifo->in == fifo->out)
        {
            fifo->full = 1;
            fifo->out++;   /* 覆盖最早的1B; in != out */

            if (fifo->out >= fifo->size)
            {
                fifo->out = 0;
            }
        }
    }

    return i;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: mfifo_get                                                        */
/* 功能描述: 从FIFO缓冲区中读取数据                                           */
/* 输入参数: fifo --- 指向FIFO结构体的指针                                    */
/*           buf  --- 存放读取数据的缓冲区的指针                              */
/*           len  --- 要读取数据的大小                                        */
/* 输出参数: 无                                                               */
/* 返 回 值: 读取数据的长度                                                   */
/* 操作流程:                                                                  */
/* 其它说明: in == out 即无数据                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int mfifo_get(struct _mfifo *fifo, unsigned char *buf, int len)
{
    int i = 0, rlen = 0;

    rlen = RING_D_VAL(fifo->in, fifo->out, fifo->size);
  
    if (rlen == 0)
    {
        return 0;
    }

    rlen = __min(rlen, len);

    for (i = 0; i < rlen; i++)
    {
        buf[i] = fifo->buffer[fifo->out++];

        if (fifo->out >= fifo->size)
        {
            fifo->out = 0;
        }
    }

    return i;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: mfifo_have                                                       */
/* 功能描述: FIFO缓冲区中是否有可读取数据                                    */
/* 输入参数: fifo --- 指向FIFO结构体的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: 无                                                               */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int mfifo_have(struct _mfifo *fifo)
{
    return (fifo->in != fifo->out) ? 1 : 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: mfifo_test                                                       */
/* 功能描述: 测试用例                                                         */
/* 输入参数: fifo --- 指向FIFO结构体的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: 无                                                               */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int mfifo_test(void)
{
    struct _mfifo fifo;
    unsigned char *pf = NULL;
    unsigned char out[16] = {0};

    pf = malloc(64);
    if (pf == NULL) return -1;

    mfifo_init(&fifo, pf, 64);
    mfifo_put(&fifo, (unsigned char *)"hello", 5);
    mfifo_get(&fifo, out, 10);

    if (pf != NULL)  free(pf);
    return 0;
}

