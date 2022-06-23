/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_xyzModem.c                                                    */
/* 内容摘要: xyzModem协议的流处理程序                                        */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-12-08                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-12-08        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "Fn_xyzModem.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
/******************************************************************************/
/* 协议控制字                                                                 */
/******************************************************************************/
#define SOH                             0x01
#define STX                             0x02
#define EOT                             0x04
#define ACK                             0x06
#define BSP                             0x08
#define NAK                             0x15
#define CAN                             0x18
#define EOF                             0x1A

/******************************************************************************/
/* xyzModem的配置信息                                                         */
/******************************************************************************/
//#define USE_YMODEM_LENGTH

#define ZYZMODEM_CHAR_TIMEOUT           2000      /* 字符输入2s超时          */
#define ZYZMODEM_MAX_RETRIES            20
#define XYZMODEM_MAX_RETRIES_WITH_CRC   10
#define XYZMODEM_CAN_COUNT              3         /* 收到3个CAN后退出        */
#define XYZMODEM_DELAY                  20

/******************************************************************************/
/* DEBUG宏定义，暂不支持                                                     */
/******************************************************************************/
#define DEBUG

#ifdef DEBUG_XYZMODEM
    #define ZM_DEBUG(x)                     x
#else
    #define ZM_DEBUG(x)
#endif

/******************************************************************************/
/*                              内部数据类型定义                             */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static void xyzModem_flush(void);
static int  xyzModem_get_hdr(void);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static  T_XYZMODEM_CB  xyz;

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/******************************************************************************/
/* 验证十六进制字符                                                          */
/******************************************************************************/
inline static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/******************************************************************************/
/* 转换单个十六进制字符值                                                    */
/******************************************************************************/
inline static int _from_hex(char c)
{
    int ret = 0;

    if ((c >= '0') && (c <= '9'))
    {
        ret = (c - '0');
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        ret = (c - 'a' + 0x0a);
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        ret = (c - 'A' + 0x0A);
    }
    else
    {
        ret = 16;
    }

    return ret;
}

/******************************************************************************/
/* 将字符转换为小写                                                          */
/******************************************************************************/
inline static char _tolower(char c)
{
    return ((c >= 'A') && (c <= 'Z')) ? ((c - 'A') + 'a') : c;
}

/******************************************************************************/
/* 将字符转换为大写                                                          */
/******************************************************************************/
inline static char _toupper(char c)
{
    return ((c >= 'a') && (c <= 'z')) ? ((c - 'a') + 'A') : c;
}

#ifdef DEBUG_XYZMODEM
static int zm_dprintf(char *fmt, ...)
{
}

static void zm_flush(void)
{
}

static void zm_dump_buf(void *buf, int len)
{
}

static void zm_new(void)
{
}

static void zm_save(unsigned char c)
{
}

static void zm_dump(int line)
{

}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_flush                                                   */
/* 功能描述: 等待进入空闲状态                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void xyzModem_flush(void)
{
    int res;
    char c;

    while (1)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, &c);
        if (!res)
        {
            return;
        }
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_get_hdr                                                 */
/* 功能描述: 接收一个数据帧                                                   */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int xyzModem_get_hdr(void)
{
    char c;
    int  i, res;
    int  hdr_found = 0;
    int  can_total;
    int  hdr_chars;
    int  cksum;

    ZM_DEBUG(zm_new());

    /* Find the start of a header */
    can_total = 0;
    hdr_chars = 0;

    if (xyz.tx_ack)
    {
        CYGACC_COMM_IF_PUTC(*xyz.__chan, ACK);
        xyz.tx_ack = 0;
    }

    while (!hdr_found)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, &c);
        ZM_DEBUG(zm_save(c));

        if (res)
        {
            hdr_chars++;

            switch (c)
            {
            case SOH:
                xyz.total_SOH++;

            case STX:
                if (c == STX)
                {
                    xyz.total_STX++;
                }

                hdr_found = 1;
                break;

            case CAN:
                xyz.total_CAN++;
                ZM_DEBUG(zm_dump(__LINE__));

                if (++can_total == XYZMODEM_CAN_COUNT)
                {
                    return XYZMODEM_CANCEL;
                }
                else
                {
                    /* Wait for multiple CAN to avoid early quits */
                    break;
                }

            case EOT:
                /* EOT only supported if no noise */
                if (hdr_chars == 1)
                {
                    CYGACC_COMM_IF_PUTC(*xyz.__chan, ACK);
                    ZM_DEBUG(zm_dprintf("ACK on EOT #%d\n", __LINE__));
                    ZM_DEBUG(zm_dump(__LINE__));
                    return XYZMODEM_EOF;
                }

            default:
                /* Ignore, waiting for start of header */
                ;
            }
        }
        else
        {
            /* Data stream timed out */
            xyzModem_flush();    /* Toss any current input */
            ZM_DEBUG(zm_dump(__LINE__));
            CYGACC_CALL_IF_DELAY_US(250000);
            return XYZMODEM_TIMEOUT;
        }
    }

    /* Header found, now read the data */
    res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, (char *)&xyz.blk);
    ZM_DEBUG(zm_save(xyz.blk));

    if (!res)
    {
        ZM_DEBUG(zm_dump(__LINE__));
        return XYZMODEM_TIMEOUT;
    }

    res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, (char *) &xyz.cblk);
    ZM_DEBUG(zm_save(xyz.cblk));

    if (!res)
    {
        ZM_DEBUG(zm_dump(__LINE__));
        return XYZMODEM_TIMEOUT;
    }

    xyz.len = (c == SOH) ? 128 : 1024;
    xyz.bufp = xyz.pkt;

    for (i = 0; i < xyz.len; i++)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, &c);
        ZM_DEBUG(zm_save(c));

        if (res)
        {
            xyz.pkt[i] = c;
        }
        else
        {
            ZM_DEBUG(zm_dump(__LINE__));
            return XYZMODEM_TIMEOUT;
        }
    }

    res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, (char *) &xyz.crc1);
    ZM_DEBUG(zm_save(xyz.crc1));
    if (!res)
    {
        ZM_DEBUG(zm_dump(__LINE__));
        return XYZMODEM_TIMEOUT;
    }

    if (xyz.crc_mode)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT(*xyz.__chan, (char *) &xyz.crc2);
        ZM_DEBUG(zm_save(xyz.crc2));

        if (!res)
        {
            ZM_DEBUG(zm_dump(__LINE__));
            return XYZMODEM_TIMEOUT;
        }
    }

    ZM_DEBUG(zm_dump(__LINE__));

    /* Validate the message */
    if ((xyz.blk ^ xyz.cblk) != (unsigned char) 0xFF)
    {
        ZM_DEBUG(zm_dprintf("Framing error - blk: %x/%x/%x\n",
                            xyz.blk, xyz.cblk, (xyz.blk ^ xyz.cblk)));
        ZM_DEBUG(zm_dump_buf(xyz.pkt, xyz.len));
        xyzModem_flush();
        return XYZMODEM_FRAME;
    }

    /* Verify checksum/CRC */
    if (xyz.crc_mode)
    {
        cksum = cyg_crc16(xyz.pkt, xyz.len);
        if (cksum != ((xyz.crc1 << 8) | xyz.crc2))
        {
            ZM_DEBUG(zm_dprintf("CRC error - recvd: %02x%02x, computed: %x\n",
                                xyz.crc1, xyz.crc2, cksum & 0xFFFF));
            return XYZMODEM_CKSUM;
        }
    }
    else
    {
        cksum = 0;

        for (i = 0; i < xyz.len; i++)
        {
            cksum += xyz.pkt[i];
        }

        if (xyz.crc1 != (cksum & 0xFF))
        {
            ZM_DEBUG(zm_dprintf("Checksum error - recvd: %x, computed: %x\n",
                                xyz.crc1, cksum & 0xFF));
            return XYZMODEM_CKSUM;
        }
    }

    /* If we get here, the message passes [structural] muster */
    return 0;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: CYGACC_COMM_IF_GETC_TIMEOUT                                      */
/* 功能描述: 在规定的时间内接收一个字符                                       */
/* 输入参数: chan --- 通道                                                    */
/*           c --- 指向存放获取到的字符的指针                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: 0-超时；1-成功                                                   */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int CYGACC_COMM_IF_GETC_TIMEOUT(char chan, char *c)
{
    int counter = 0;

    while (!tstc() && (counter < ZYZMODEM_CHAR_TIMEOUT * 1000 / XYZMODEM_DELAY))
    {
        CYGACC_CALL_IF_DELAY_US(XYZMODEM_DELAY);
        counter++;
    }

    if (tstc())
    {
        *c = getc();
        return 1;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: CYGACC_COMM_IF_PUTC                                              */
/* 功能描述: 输出一个字符                                                     */
/* 输入参数: x --- 通道                                                       */
/*           y --- 要输出的字符                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void CYGACC_COMM_IF_PUTC(char x, char y)
{
    putc(y);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_parse_num                                               */
/* 功能描述: 从指定的字符串中解析出数字                                       */
/* 输入参数: s     --- 指向字符串的指针                                       */
/*           delim --- 指向分隔字符串的指针                                   */
/* 输出参数: val   --- 指向存放解析得到的数值的指针                           */
/*           es    --- 解析后指向字符串的位置                                 */
/* 返 回 值: 1-解析成功；0-解析失败                                    */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int xyzModem_parse_num(char *s, char *delim, int *val, char **es)
{
    int first = 1;
    int  radix = 10;
    char c     = 0;
    int  digit;
    int result = 0;

    while (*s == ' ')
    {
        s++;
    }

    while (*s)
    {
        if (first && (s[0] == '0') && (_tolower(s[1]) == 'x'))
        {
            radix = 16;
            s += 2;
        }

        first = 0;
        c = *s++;

        if (_is_hex(c) && ((digit = _from_hex(c)) < radix))
        {
            result = (result * radix) + digit;
        }
        else
        {
            if (delim != (char *)0)
            {
                /* See if this character is one of the delimiters */
                char *dp = delim;

                while (*dp && (c != *dp))
                {
                    dp++;
                }

                if (*dp)
                {
                    break;      /* Found a good delimiter */
                }
            }

            return 0;     /* Malformatted number */
        }
    }

    *val = result;

    if (es != (char **) 0)
    {
        *es = s;
    }

    return 1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_stream_open                                             */
/* 功能描述: 打开传输流，初始化全局数据，并接收起始帧数据                     */
/* 输入参数: info --- 指向连接信息的指针                                      */
/* 输出参数: err  --- 指向错误信息的指针                                      */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int xyzModem_stream_open(T_XYZMODEM_CNCT_INFO *info, int *err)
{
    int stat        = 0;
    int retries     = ZYZMODEM_MAX_RETRIES;
    int crc_retries = XYZMODEM_MAX_RETRIES_WITH_CRC;
    int dummy       = 0;

#ifdef XYZMODEM_Z
    if (info->mode == XYZMODEM_Z)
    {
        *err = XYZMODEM_NO_ZMODEM;
        return -1;
    }
#endif

    xyz.__chan        = &dummy;
    xyz.len           = 0;
    xyz.crc_mode      = 1;
    xyz.at_eof        = 0;
    xyz.tx_ack        = 0;
    xyz.mode          = info->mode;
    xyz.total_retries = 0;
    xyz.total_SOH     = 0;
    xyz.total_STX     = 0;
    xyz.total_CAN     = 0;
#ifdef USE_YMODEM_LENGTH
    xyz.read_length   = 0;
    xyz.file_length   = 0;
#endif

    CYGACC_COMM_IF_PUTC(*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

    if (xyz.mode == XYZMODEM_X)
    {
        /* X-modem doesn't have an information header - exit here */
        xyz.next_blk = 1;
        return 0;
    }

    while (retries-- > 0)
    {
        stat = xyzModem_get_hdr();

        if (stat == 0)
        {
            /* Y-modem file information header */
            if (xyz.blk == 0)
            {
#ifdef USE_YMODEM_LENGTH
                /* skip filename */
                while (*xyz.bufp++);
                /* get the length */
                xyzModem_parse_num((char *)xyz.bufp, &xyz.file_length, NULL, " ");
#endif
                /* The rest of the file name data block quietly discarded */
                xyz.tx_ack = 1;
            }

            xyz.next_blk = 1;
            xyz.len = 0;
            return 0;
        }
        else if (stat == XYZMODEM_TIMEOUT)
        {
            if (--crc_retries <= 0)
            {
                xyz.crc_mode = 0;
            }

            CYGACC_CALL_IF_DELAY_US(5 * 100000); /* Extra delay for startup */
            CYGACC_COMM_IF_PUTC(*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
            xyz.total_retries++;

            ZM_DEBUG(zm_dprintf("NAK (%d)\n", __LINE__));
        }

        if (stat == XYZMODEM_CANCEL)
        {
            break;
        }
    }

    *err = stat;
    ZM_DEBUG(zm_flush());
    return -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_stream_read                                             */
/* 功能描述: 从传输流中读取数据帧                                             */
/* 输入参数: buf --- 指向缓存的指针                                           */
/*           size --- 缓存区的大小                                            */
/* 输出参数: err --- 指向错误信息的指针                                       */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int xyzModem_stream_read(char *buf, int size, int *err)
{
    int stat;
    int total;
    int len;
    int retries;

    total = 0;
    stat  = XYZMODEM_CANCEL;

    /* Try and get 'size' bytes into the buffer */
    while (!xyz.at_eof && (size > 0))
    {
        if (xyz.len == 0)
        {
            retries = ZYZMODEM_MAX_RETRIES;

            while (retries-- > 0)
            {
                stat = xyzModem_get_hdr();

                if (stat == 0)
                {
                    if (xyz.blk == xyz.next_blk)
                    {
                        xyz.tx_ack = 1;
                        ZM_DEBUG(zm_dprintf("ACK block %d (%d)\n",
                                            xyz.blk, __LINE__));
                        xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

#if defined(XYZMODEM_Z) || defined(USE_YMODEM_LENGTH)
                        if (xyz.mode == XYZMODEM_X || xyz.file_length == 0)
                        {
#else
                        if (1)
                        {
#endif
                            /* Data blocks can be padded with ^Z (EOF) characters */
                            /* This code tries to detect and remove them */
                            if ((xyz.bufp[xyz.len - 1] == EOF) &&
                                    (xyz.bufp[xyz.len - 2] == EOF) &&
                                    (xyz.bufp[xyz.len - 3] == EOF))
                            {
                                while (xyz.len && (xyz.bufp[xyz.len - 1] == EOF))
                                {
                                    xyz.len--;
                                }
                            }
                        }

#ifdef USE_YMODEM_LENGTH
                        /*
                        * See if accumulated length exceeds that of the file.
                        * If so, reduce size (i.e., cut out pad bytes)
                        * Only do this for Y-modem (and Z-modem should it ever
                        * be supported since it can fall back to Y-modem mode).
                        */
                        if (xyz.mode != XYZMODEM_X && 0 != xyz.file_length)
                        {
                            xyz.read_length += xyz.len;

                            if (xyz.read_length > xyz.file_length)
                            {
                                xyz.len -= (xyz.read_length - xyz.file_length);
                            }
                        }
#endif
                        break;
                    }
                    else if (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
                    {
                        /* Just re-ACK this so sender will get on with it */
                        CYGACC_COMM_IF_PUTC(*xyz.__chan, ACK);
                        continue; /* Need new header */
                    }
                    else
                    {
                        stat = XYZMODEM_SEQUENCE;
                    }
                }

                if (stat == XYZMODEM_CANCEL)
                {
                    break;
                }

                if (stat == XYZMODEM_EOF)
                {
                    CYGACC_COMM_IF_PUTC(*xyz.__chan, ACK);
                    ZM_DEBUG(zm_dprintf("ACK (%d)\n", __LINE__));

                    if (xyz.mode == XYZMODEM_Y)
                    {
                        CYGACC_COMM_IF_PUTC(*xyz.__chan,
                                            (xyz.crc_mode ? 'C' : NAK));
                        xyz.total_retries++;
                        ZM_DEBUG(zm_dprintf("Reading Final Header\n"));

                        stat = xyzModem_get_hdr();
                        CYGACC_COMM_IF_PUTC(*xyz.__chan, ACK);
                        ZM_DEBUG(zm_dprintf("FINAL ACK (%d)\n", __LINE__));
                    }

                    xyz.at_eof = 1;
                    break;
                }

                CYGACC_COMM_IF_PUTC(*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
                xyz.total_retries++;
                ZM_DEBUG(zm_dprintf("NAK (%d)\n", __LINE__));
            }

            if (stat < 0)
            {
                *err = stat;
                xyz.len = -1;
                return total;
            }
        }

        /* Don't "read" data from the EOF protocol package */
        if (!xyz.at_eof)
        {
            len = xyz.len;

            if (size < len)
            {
                len = size;
            }

            memcpy(buf, xyz.bufp, len);

            size  -= len;
            buf   += len;
            total += len;

            xyz.len  -= len;
            xyz.bufp += len;
        }
    }

    return total;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_stream_close                                            */
/* 功能描述: 关闭传输流                                                       */
/* 输入参数: err --- 指向错误信息的指针                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void xyzModem_stream_close(int *err)
{
    mprintf("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, "
            "%d retries\r\n",
            xyz.crc_mode ? "CRC" : "Cksum",
            xyz.total_SOH,
            xyz.total_STX,
            xyz.total_CAN,
            xyz.total_retries);

    ZM_DEBUG(zm_flush());
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_stream_terminate                                        */
/* 功能描述: 终止传输流                                                       */
/* 输入参数: abort --- 是否取消传输                                           */
/*           getc --- 指向获取字符函数的指针                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 需要能够清除输入缓冲区，所以必须使用getc                        */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void xyzModem_stream_terminate(int abort, int (*getc)(void))
{
    if (abort)
    {
        ZM_DEBUG(zm_dprintf("!!!! TRANSFER ABORT !!!!\r\n"));

        switch (xyz.mode)
        {
        case XYZMODEM_X:
        case XYZMODEM_Y:
            /* The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal */
            /* number of Backspaces is a friendly way to get the other end to abort. */
            CYGACC_COMM_IF_PUTC(*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, BSP);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, BSP);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, BSP);
            CYGACC_COMM_IF_PUTC(*xyz.__chan, BSP);

            /* Now consume the rest of what's waiting on the line. */
            ZM_DEBUG(zm_dprintf("Flushing serial line.\n"));
            xyzModem_flush();
            xyz.at_eof = 1;
            break;

#ifdef XYZMODEM_Z
        case XYZMODEM_Z:
            /* Might support it some day I suppose. */
            break;
#endif
        }
    }
    else
    {
        ZM_DEBUG(zm_dprintf("Engaging cleanup mode...\n"));
        /*
        * Consume any trailing crap left in the inbuffer from
        * previous received blocks. Since very few files are an exact multiple
        * of the transfer block size, there will almost always be some gunk here.
        * If we don't eat it now, RedBoot will think the user typed it.
        */
        ZM_DEBUG(zm_dprintf("Trailing gunk:\n"));
        while ((*getc)() > -1);
        ZM_DEBUG(zm_dprintf("\n"));
        /*
        * Make a small delay to give terminal programs like minicom
        * time to get control again after their file transfer program
        * exits.
        */
        CYGACC_CALL_IF_DELAY_US(250000);
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: xyzModem_error                                                   */
/* 功能描述: 获取错误码对应的字符串描述                                      */
/* 输入参数: err --- 错误码                                                   */
/* 输出参数: 无                                                               */
/* 返 回 值: char                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-08              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
char *xyzModem_error(int err)
{
    switch (err)
    {
    case XYZMODEM_ACCESS:
        return "Can't access file";

    case XYZMODEM_NO_ZMODEM:
        return "Sorry, zModem not available yet";

    case XYZMODEM_TIMEOUT:
        return "Timed out";

    case XYZMODEM_EOF:
        return "End of file";

    case XYZMODEM_CANCEL:
        return "Cancelled";

    case XYZMODEM_FRAME:
        return "Invalid framing";

    case XYZMODEM_CKSUM:
        return "Checksum error";

    case XYZMODEM_SEQUENCE:
        return "Block sequence error";

    default:
        return "Unknown error";
    }
}

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
