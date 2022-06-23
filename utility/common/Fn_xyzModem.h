/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_xyzModem.h                                                    */
/* 内容摘要: xyzModem协议的流处理程序                                         */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-12-08                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-12-08        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_XYZMODEM_H
#define FN_XYZMODEM_H


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
#define XYZMODEM_PACK                1024

#define XYZMODEM_X                   1
#define XYZMODEM_Y                   2
#define XYZMODEM_Z                   3 

#define XYZMODEM_CLOSE               1
#define XYZMODEM_ABORT               2
#define XYZMODEM_ACCESS             -1
#define XYZMODEM_NO_ZMODEM          -2
#define XYZMODEM_TIMEOUT            -3
#define XYZMODEM_EOF                -4
#define XYZMODEM_CANCEL             -5
#define XYZMODEM_FRAME              -6
#define XYZMODEM_CKSUM              -7
#define XYZMODEM_SEQUENCE           -8

#define CYGACC_CALL_IF_DELAY_US(x)  delay_us(x)

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
/*<STRUCT+>********************************************************************/
/* 结构: T_XYXMODEM_CB                                                        */
/* 注释: xyzmodem协议的控制块                                                 */
/*<STRUCT->********************************************************************/
typedef struct t_xyxmodem_cb
{
    int    *__chan;
    char    pkt[XYZMODEM_PACK];
    char   *bufp;
    int     blk;
    int     cblk;
    int     crc1;
    int     crc2;
    int     next_blk;
    int     len;
    int     mode;
    int     total_retries;
    int     total_SOH;
    int     total_STX;
    int     total_CAN;
    int     crc_mode;
    int     at_eof;
    int     tx_ack;
    
#ifdef USE_YMODEM_LENGTH
    int     file_length;
    int     read_length;
#endif
} T_XYZMODEM_CB;

/*<STRUCT+>********************************************************************/
/* 结构: T_XYZMODEM_CNCT_INFO                                                 */
/* 注释: XYZMODEM连接信息结构体                                              */
/*<STRUCT->********************************************************************/
typedef struct t_xyzmodem_cnct_info
{
    char   *filename;
    int     mode;
    int     chan;
    
} T_XYZMODEM_CNCT_INFO;

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int   xyzModem_stream_open(T_XYZMODEM_CNCT_INFO *info, int *err);
void  xyzModem_stream_close(int *err);
void  xyzModem_stream_terminate(int method, int (*getc)(void));
int   xyzModem_stream_read(char *buf, int size, int *err);
char *xyzModem_error(int err);

#ifdef __cplusplus
    }
#endif

#endif


