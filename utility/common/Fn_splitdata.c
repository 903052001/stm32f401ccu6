/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.              */
/*                                                                            */
/* 文件名称: Fn_splitdata.c                                                   */
/* 内容摘要: 合分包处理源文件                                                */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-04-13                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-04-13        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Fn_check.h"
#include "Fn_splitdata.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define dbug              printf

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int Fn_SplitData_GetSize(const unsigned char *pData, T_SPLIT_L length);
static int Fn_SplitData_GetIdx(T_SPLITDATA *DataStruct, const unsigned char *pData);
static int Fn_SplitData_Align(T_SPLITDATA *HandleStruct, unsigned char *pData);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
T_SPLITDATA    g_tSplitData = {0};                      /* 合分包操作实体    */
unsigned char  g_tSplitBuf[SPLIT_LENGTH_MAX] = {0};     /* 粘包数据区        */

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_SplitData_GetSize                                             */
/* 功能描述: 获取当前有效数据包的长度                                        */
/* 输入参数: pData --- 过滤的数据                                             */
/*           length --- 相对应的长度信息                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-13              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int Fn_SplitData_GetSize(const unsigned char *pData, T_SPLIT_L length)
{
    int i, ret = 0;

    switch (length.length_type)
    {
    case bigEN:
        for (i = 0; i < length.length_count; i++)
        {
            ret |= (*(pData + length.length_location + i) << ((length.length_count - i - 1) * 8));
        }
        break;

    case littleEN:
        for (i = 0; i < length.length_count; i++)
        {
            ret |= (*(pData + length.length_location + i) >> ((length.length_count - i - 1) * 8));
        }
        break;

    default:
        dbug("\r\ncase error: file %s on line %d.\r\n", __FILE__, __LINE__);
        break;
    }

    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_SplitData_GetIdx                                              */
/* 功能描述: 获取帧头出现位置的指针,同时检测此包数据是否含有有效数据头      */
/* 输入参数: DataStruct --- 操作句柄                                          */
/*           pData --- 筛选的数据源                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-13              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int Fn_SplitData_GetIdx(T_SPLITDATA *DataStruct, const unsigned char *pData)
{
    int i, ret = -1;

    switch (DataStruct->header_length)
    {
    case 1:
        for (i = 0; i < DataStruct->iBufCompSize; i++)
        {
            if (*(pData + i) == (DataStruct->header[0]))
            {
                ret = i;
                break;
            }
        }
        break;

    case 2:
        for (i = 0; i < DataStruct->iBufCompSize; i++)
        {
            if ((*(pData + i) == (DataStruct->header[0])) && (*(pData + i + 1) == (DataStruct->header[1])))
            {
                ret = i;
                break;
            }
        }
        break;

    case 3:
        for (i = 0; i < DataStruct->iBufCompSize; i++)
        {
            if ((*(pData + i) == (DataStruct->header[0])) && (*(pData + i + 1) == (DataStruct->header[1])) && (*(pData + i + 2) == (DataStruct->header[2])))
            {
                ret = i;
                break;
            }
        }
        break;

    case 4:
        for (i = 0; i < DataStruct->iBufCompSize; i++)
        {
            if ((*(pData + i) == (DataStruct->header[0])) && (*(pData + i + 1) == (DataStruct->header[1])) && (*(pData + i + 2) == (DataStruct->header[2])) && (*(pData + i + 3) == (DataStruct->header[3])))
            {
                ret = i;
                break;
            }
        }
        break;

    default:
        dbug("\r\ncase error: file %s on line %d.\r\n", __FILE__, __LINE__);
        break;
    }

    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_SplitData_Align                                               */
/* 功能描述: 帧头对齐操作函数                                                */
/* 输入参数: HandleStruct --- 操作句柄                                        */
/*           pData --- 本次接收的包长度                                       */
/* 输出参数: <0:索引帧头失败; >0该帧头数据包长度                             */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 只负责对齐操作                                                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-13              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int Fn_SplitData_Align(T_SPLITDATA *HandleStruct, unsigned char *pData)
{
    int result = -1;

    /* 从缓存区的开头索引当前缓存区内的所有数据 */
    result = Fn_SplitData_GetIdx(HandleStruct, pData);

    if (result < 0)  /* 此缓存中不包含帧头信息 */
    {
        HandleStruct->iBufCompSize = 0;  /* 清空缓存 */
    }
    else
    {
        /* 重置当前缓存区中的数据个数 */
        HandleStruct->iBufCompSize -= result;
        /* 从帧头开始挪到了缓存区开头 */
        memmove(pData, pData + result, HandleStruct->iBufCompSize);
    }

    return result;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_SplitData_TerminalRegister                                    */
/* 功能描述: 合分包功能初始化                                                 */
/* 输入参数: pBuf --- 缓存数据地址                                            */
/*           iBufSize --- 单次缓存数据的最大长度                              */
/*           iheader --- 帧头                                                 */
/*           ihearder_length --- 帧头长度                                     */
/*           ilength_location --- 长度在包中相对位置(注意:起始下标位置为0)   */
/*           ilength_count --- 表示该长度的字节数                             */
/*           ilength_mini_effe --- 表示指令最小有效程度(必须>1)               */
/*           iFunCB --- 回调函数                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: T_SPLITDATA                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-13              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
T_SPLITDATA Fn_SplitData_TerminalRegister(unsigned char *pBuf, unsigned short iBufSize,
        unsigned char *iheader, unsigned char ihearder_length,
        unsigned char  ilength_location, unsigned char ilength_count,
        unsigned char  ilength_mini_effe, SPLIT_CB_FUN iFunCB)
{
    T_SPLITDATA  InitStruct;
    T_SPLITDATA  *InitEroo = NULL;

    union
    {
        unsigned int  i;
        unsigned char s[4];
    } c;

    c.i = 0x12345678;

    if (iheader == NULL)
    {
        return *InitEroo;
    }

    if (ihearder_length < SPLIT_HEAD_MAX)
    {
        memcpy(InitStruct.header, iheader, ihearder_length);
        InitStruct.header_length = ihearder_length;
    }
    else
    {
        return *InitEroo;
    }

    InitStruct.pBuf = pBuf;
    InitStruct.length.length_location = ilength_location;
    InitStruct.length.length_count = ilength_count;
    if (ilength_mini_effe > 0)
    {
        InitStruct.length.length_mini_effe = ilength_mini_effe;
    }
    else
    {
        return *InitEroo;
    }

    InitStruct.length.length_type = (0x12 == c.s[0]) ? 1 : 0;
    InitStruct.fnCallback = iFunCB;
    InitStruct.iBufSize_max = iBufSize;
    InitStruct.iBufCompSize = 0;

    memset(InitStruct.pBuf, 0, InitStruct.iBufSize_max);
    return InitStruct;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_SplitData_Handle                                              */
/* 功能描述: 粘包的具体实现过程                                               */
/* 输入参数: DataStruct --- 操作句柄                                          */
/*           pRcvData --- 输入源数据(剔除URC头和\r\n)                         */
/*           iRcvSize --- 源数据长度                                          */
/*           bUrgent --- 是否为紧急数据(!0紧急)                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-14              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_SplitData_Handle(T_SPLITDATA *DataStruct, const unsigned char *pRcvData, int iRcvSize, int bUrgent)
{
    unsigned char   *pStart        = NULL;
    unsigned short   iBufAvailSize = 0;
    unsigned short   pack_length   = 0;

    /**************************************************************************/
    /* 将当前数据拼接到上次数据尾,溢出缓存区的数据将被丢弃(pBuf恒指向数据头)*/
    /**************************************************************************/
    iBufAvailSize = DataStruct->iBufSize_max - DataStruct->iBufCompSize;
    if (iRcvSize <= iBufAvailSize)
    {
        memcpy((DataStruct->pBuf + DataStruct->iBufCompSize), pRcvData, iRcvSize);
        DataStruct->iBufCompSize += iRcvSize;
    }
    else
    {
        memcpy((DataStruct->pBuf + DataStruct->iBufCompSize), pRcvData, iBufAvailSize);
        DataStruct->iBufCompSize = DataStruct->iBufSize_max;
    }

    /**************************************************************************/
    /* 标记缓存区开始操作位置                                                */
    /**************************************************************************/
    pStart = DataStruct->pBuf;

    while (DataStruct->iBufCompSize >= DataStruct->length.length_mini_effe)
    {
        if (Fn_SplitData_Align(DataStruct, pStart) >= 0)
        {
            pack_length = Fn_SplitData_GetSize(pStart, DataStruct->length);
            if ((pack_length > 0) && (pack_length <= DataStruct->iBufCompSize))
            {
                //if(pStart[pack_length-1] == CRC_XOR(pStart, pack_length-1))
                {
                    /* 接收完整一包后调用回调 */
                    (DataStruct->fnCallback)(0, pStart, pack_length, bUrgent);
                    DataStruct->iBufCompSize -= pack_length;
                    pStart += pack_length;
                    dbug("\r\ndata frame is ok!\r\n");
                }
                // else
                {
                    /* 不减pack_length是为了防止上一包是残包 */
                    DataStruct->iBufCompSize -= DataStruct->header_length;
                    pStart += DataStruct->header_length;
                    dbug("\r\ndata frame is error!\r\n");
                }
            }
            else if ((pack_length == 0) || (pack_length > DataStruct->iBufSize_max))
            {
                /* 丢弃有问题帧头 */
                DataStruct->iBufCompSize -= DataStruct->header_length;
                pStart += DataStruct->header_length;
            }
            else  /* 等同于(pack_length < 0) || (pack_length > DataStruct->iBufCompSize) */
            {
                break;   /* 数据帧不完整 */
            }
        }
    }

    if (DataStruct->iBufCompSize > 0)
    {
        memmove(DataStruct->pBuf, pStart, DataStruct->iBufCompSize);
        memset(DataStruct->pBuf + DataStruct->iBufCompSize, 0, DataStruct->iBufSize_max - DataStruct->iBufCompSize);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_SplitData_Test                                                */
/* 功能描述: 测试用例                                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明:                                                                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-14              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_SplitData_Test(void)
{
    unsigned char head[1] = {0x55};    /* 帧头 */
    /* 55 21 6E 16 AD 47 00 8A */
    g_tSplitData = Fn_SplitData_TerminalRegister(g_tSplitBuf, SPLIT_LENGTH_MAX, head, 1, 6, 2, 4, NULL);
    dbug("%#x ", g_tSplitData);
    return;
}

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
