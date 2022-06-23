/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_ymodem.h                                                      */
/* 内容摘要: ymodem传输协议的头文件                                          */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-20                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-20        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_YMODEM_H
#define FN_YMODEM_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/


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


/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
typedef int (*fREAD)(unsigned char *pBuf, int xBufSize, int xOffset);

#pragma pack(1)

/*<STRUCT+>********************************************************************/
/* 结构: T_yModem_STREAM                                                      */
/* 注释: YModem传输流结构定义                                                 */
/*<STRUCT->********************************************************************/
typedef struct t_ymdm_stream
{
    char          cPkt[1024];                   /* 数据包的缓存区             */
    char         *pPkt;                         /* 缓存区的操作指针           */
    int           xPktLen;                      /* 缓存区内单帧数据长度       */
    unsigned char ucBlk;                        /* 块编号                     */
    unsigned char ucCBlk;                       /* 块编号的反码               */
    unsigned char ucCRC1;                       /* CRC校验高字节或校验数据和  */
    unsigned char ucCRC2;                       /* CRC校验低字节              */
    unsigned char ucNextBlk;                    /* 下一个块编号               */
    char          sFileName[32];                /* 传输文件的名称             */
    int           xFileLength;                  /* 传输文件的长度             */
    int           xReadLength;                  /* 已读取长度                 */
    int           xCRCMode;                     /* 是否采用CRC模式            */
    int           xIsTxACK;                     /* 是否需要发送ACK            */
    int           xIsEOT;                       /* 是否传输结束               */
    int           xTotalRetries;                /* 重试总次数                 */
    int           xTotalSOH;                    /* SOH包总数                  */
    int           xTotalSTX;                    /* STX包总数                  */
    int           xTotalCAN;                    /* CAN次数                    */

} T_yModem_STREAM;

#pragma pack()

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int  yModem_ReceiveOpen(T_yModem_STREAM *ptStream);
int  yModem_ReceiveRead(T_yModem_STREAM *ps, unsigned char *pRdBuf, int xBufSize);
void yModem_Terminate(T_yModem_STREAM *ps, int xIsAbort);
int  yModem_SendFile(const char *sFileName, int xFileSize, fREAD fnRd, unsigned char *pRdBuf, int xRdBufSize);

#ifdef __cplusplus
}
#endif

#endif

