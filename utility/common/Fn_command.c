/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_command.c                                                     */
/* 内容摘要: 命令行实现源文件(shell)                                         */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-20                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-20        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "drv_console.h"
#include "Fn_format.h"
#include "Fn_ymodem.h"
#include "Fn_common.h"
#include "Fn_command.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/
extern int command$$Length;
extern int command$$Base;

/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define DEBUG_PARSER
#define CONFIG_SYS_LOAD_ADDR        EFLASH_IAP_START     /* 系统加载地址     */
#define CONFIG_SYS_PROMPT           "boot > "            /* 命令行提示符     */
#define CONFIG_SYS_CBSIZE           128                  /* 命令行字符串大小 */

#define OUTSIDE_FLASH               "w25q64"            /* 外部FLASH设备名称 */
#define INSIDE_FLASH                "flash"             /* 内部FLASH设备名称 */

#define IS_BLANK(C)                (((C) == ' ') || ((C) == '\t'))
#define IS_ASCII(C)                (((unsigned char)(C)) <= 0x7f)
#define TO_ASCII(C)                (((unsigned char)(C)) & 0x7f)

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int getc_xmodem(void);
static char *delete_char(char *buffer, char *p, int *colp, int *np, int plen);
static int buildin_run_command(const char *cmd, int flag);
static int cmd_call(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[]);
static int load_serial_ymodem(unsigned int offset);

#ifdef CONFIG_AUTO_COMPLETE
static int  complete_cmdv(int argc, char *const argv[], char last_char, int maxv, char *cmdv[]);
static int  make_argv(char *s, int argvsz, char *argv[]);
static int  find_common_prefix(char *const argv[]);
static void print_argv(const char *banner, const char *leader, const char *sep, int linemax, char *const argv[]);
#endif

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static int   g_IsCtrlcDisable = 0;                  /* 禁用Ctrl^C按键        */
static int   g_IsCtrlcPressed = 0;                  /* 按下Ctrl^C按键        */
static const char erase_seq[] = "\b \b";            /* 擦除序列              */
static const char   tab_seq[] = "        ";         /* 用于扩展TAB按键       */
static char console_buffer[CONFIG_SYS_CBSIZE + 1];  /* 控制台I/O缓冲区       */
static char lastcommand[CONFIG_SYS_CBSIZE] = {0};   /* 用于备份输入的命令    */

#ifdef CONFIG_AUTO_COMPLETE
static char copy_buffer[CONFIG_SYS_CBSIZE] = {0};   /* 控制台I/O缓冲区的备份 */
#endif

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: delete_char                                                      */
/* 功能描述: 从缓存区中删除一个字符,并更新缓存区的指针,列(下标)位置,        */
/*           以及缓存区的字符数量                                             */
/* 输入参数: buffer --- 指向缓存区的指针                                      */
/*           p --- 缓存区当前的位置指针                                       */
/*           colp --- 指向缓存区列数(下标)的指针                              */
/*           np --- 指向缓存区字符数量的指针                                  */
/*           plen --- 提示符的长度                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: 指向删除字符后的缓存区位置指针                                  */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static char *delete_char(char *buffer, char *p, int *colp, int *np, int plen)
{
    char *s;

    if (*np == 0)
    {
        return (p);
    }

    if (*(--p) == '\t')                    /* 将重新键入整行 */
    {
        while (*colp > plen)
        {
            dbug(erase_seq);
            (*colp)--;
        }

        for (s = buffer; s < p; ++s)
        {
            if (*s == '\t')
            {
                dbug(tab_seq + ((*colp) & 7));
                *colp += 8 - ((*colp) & 7);
            }
            else
            {
                ++(*colp);
                dbug("%c", *s);
            }
        }
    }
    else
    {
        dbug(erase_seq);
        (*colp)--;
    }

    (*np)--;

    return (p);
}



/*<FUNC+>**********************************************************************/
/* 函数名称: ctrlc                                                            */
/* 功能描述: 探测是否接收到了Ctrl+C(^C = 0x03,正文结束)字符                  */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int ctrlc(void)
{
    if (!g_IsCtrlcDisable)
    {
        if (bsp_console_havec())
        {
            switch (bsp_console_getc())
            {
            case 0x03:    /* ^C */
                g_IsCtrlcPressed = 1;
                return 1;

            default:
                break;
            }
        }
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: disable_ctrlc                                                    */
/* 功能描述: 禁止ctrlc()的检测                                                */
/* 输入参数: disable --- ctrlc()检测功能的状态，0-使能；1-禁止               */
/* 输出参数: 无                                                               */
/* 返 回 值: 上一个状态                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int disable_ctrlc(int disable)
{
    int prev = g_IsCtrlcDisable;

    g_IsCtrlcDisable = disable;
    
    return prev;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: had_ctrlc                                                        */
/* 功能描述: 获取是否正在处理ctrlc()的状态                                   */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int had_ctrlc(void)
{
    return g_IsCtrlcPressed;
}



/*<FUNC+>**********************************************************************/
/* 函数名称: buildin_run_command                                              */
/* 功能描述: 执行内建的执行命令函数                                          */
/* 输入参数: cmd --- 指向命令的字符串                                        */
/*           flag --- 标志位                                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: 1 - 命令已执行,可重复                                           */
/*           0 - 命令已执行,但不可重复;中断的命令总是被认为是不可重复的     */
/*          -1 - 未执行(无法识别或参数太多)                                  */
/*                (如果cmd为NULL或""或长于CONFIG_SYS_CBSIZE-1, 则视为未识别) */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int buildin_run_command(const char *cmd, int flag)
{
    char  cmdbuf[CONFIG_SYS_CBSIZE];    /* working copy of cmd                */
    char *token;                        /* start of token in cmdbuf           */
    char *sep;                          /* end of token (separator) in cmdbuf */
    char *str = cmdbuf;
    char *argv[CONFIG_SYS_MAXARGS + 1]; /* add a NULL terminated              */
    int   argc;
    int   inquotes;
    int   repeatable = 1;
    int   rc = 0;

#ifdef DEBUG_PARSER
    dbug("[RUN_COMMAND] cmd[%p]: ", cmd);
    dbug(cmd ? cmd : "NULL");
    dbug("\r\n");
#endif

    g_IsCtrlcPressed = 0;               /* 清除Ctrl^C的处理状态 */

    /**************************************************************************/
    /* 检查命令行字符串的有效性                                              */
    /**************************************************************************/
    if (!cmd || !*cmd)
    {
        return -1;
    }

    /**************************************************************************/
    /* 检查命令行字符串的长度                                                */
    /**************************************************************************/
    if (strlen(cmd) >= CONFIG_SYS_CBSIZE)
    {
        dbug("Command too long\r\n");
        return -2;
    }

    strcpy(cmdbuf, cmd);

#ifdef DEBUG_PARSER
    dbug("[PROCESS_SEPARATORS]: %s\r\n", cmd);
#endif

    while (*str)
    {
        /*************************************************************************/
        /* 查找字符串中的分隔符或结束符;允许通过"\;"的形式在一行中包含多个命令 */
        /*************************************************************************/
        for (inquotes = 0, sep = str; *sep; sep++)
        {
            if ((*sep == '\'') && (*(sep - 1) != '\\'))
            {
                inquotes = !inquotes;
            }

            if (!inquotes && (*sep == ';') && (sep != str) && (*(sep - 1) != '\\'))
            {
                break;
            }
        }

        /**********************************************************************/
        /* 确定当前需要执行的命令行字符串                                    */
        /**********************************************************************/
        token = str;

        if (*sep)
        {
            str = sep + 1;                  /* start of command for next pass */
            *sep = '\0';
        }
        else
        {
            str = sep;                      /* no more commands for next pass */
        }

#ifdef DEBUG_PARSER
        dbug("token: \"%s\"\r\n", token);
#endif

        /**********************************************************************/
        /* 提取命令行参数                                                     */
        /**********************************************************************/
        if ((argc = parse_line(token, argv)) == 0)
        {
            rc = -1;
            continue;
        }

        /**********************************************************************/
        /* 执行命令行,返回-1                                                 */
        /**********************************************************************/
        if (0 != cmd_process(flag, argc, argv, &repeatable))
        {
            rc = -1;
        }

        /**********************************************************************/
        /* 获取是否正在处理Ctrl^C                                            */
        /**********************************************************************/
        if (g_IsCtrlcPressed)
        {
            return -3;                      /* if stopped then not repeatable */
        }
    }

    return rc ? rc : repeatable;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: cmd_call                                                         */
/* 功能描述: 调用命令执行函数                                                 */
/* 输入参数: cmdtp --- 指向命令表项的指针                                     */
/*           flag --- 标志位                                                  */
/*           argc --- 参数个数                                                */
/*           argv[] --- 指向参数列表的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int cmd_call(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[])
{
    int result;

    result = (cmdtp->cmd)(cmdtp, flag, argc, argv);

    if (result)
    {
        dbug("failed\r\n");
    }

    return result;
}

#ifdef CONFIG_AUTO_COMPLETE
static int complete_cmdv(int argc, char *const argv[], char last_char, int maxv, char *cmdv[])
{
    T_CMD_TBL  *cmdtp;
    const char *p;
    int len;
    int clen;
    int n_found = 0;
    const char *cmd;

    /* sanity? */
    if (maxv < 2)
    {
        return -2;
    }

    cmdv[0] = NULL;

    if (argc == 0)
    {
        /* output full list of commands */
        for (cmdtp = &command$$Base; cmdtp != &command$$Base + &command$$Length; cmdtp++)
        {
            if (n_found >= maxv - 2)
            {
                cmdv[n_found++] = "...";
                break;
            }

            cmdv[n_found++] = cmdtp->name;
        }

        cmdv[n_found] = NULL;

        return n_found;
    }

    /* more than one arg or one but the start of the next */
    if (argc > 1 || (last_char == '\0' || IS_BLANK(last_char)))
    {
        cmdtp = find_cmd(argv[0]);

        if (cmdtp == NULL || cmdtp->complete == NULL)
        {
            cmdv[0] = NULL;
            return 0;
        }

        return (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
    }

    cmd = argv[0];

    /*
     * Some commands allow length modifiers (like "cp.b");
     * compare command name only until first dot.
     */
    p = strchr(cmd, '.');

    if (p == NULL)
    {
        len = strlen(cmd);
    }
    else
    {
        len = p - cmd;
    }

    /* return the partial matches */
    for (cmdtp = &command$$Base; cmdtp != &command$$Base + &command$$Length; cmdtp++)
    {
        clen = strlen(cmdtp->name);

        if (clen < len)
        {
            continue;
        }

        if (memcmp(cmd, cmdtp->name, len) != 0)
        {
            continue;
        }

        /* too many! */
        if (n_found >= maxv - 2)
        {
            cmdv[n_found++] = "...";
            break;
        }

        cmdv[n_found++] = cmdtp->name;
    }

    cmdv[n_found] = NULL;

    return n_found;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: make_argv                                                        */
/* 功能描述: 依据输入的字符串,构造命令行的参数                               */
/* 输入参数: s --- 命令行字符串                                               */
/*           argvsz --- 最大参数个数                                          */
/*           argv[] --- 指向参数列表的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int make_argv(char *s, int argvsz, char *argv[])
{
    int argc = 0;

    while (argc < argvsz - 1)
    {
        while (IS_BLANK(*s))                    /* skip any white space       */
        {
            ++s;
        }

        if (*s == '\0')                         /* end of s, no more args     */
        {
            break;
        }

        argv[argc++] = s;                       /* begin of argument string   */

        while (*s && !IS_BLANK(*s))             /* find end of string         */
        {
            ++s;
        }

        if (*s == '\0')                         /* end of s, no more args     */
        {
            break;
        }

        *s++ = '\0';                            /* terminate current arg      */
    }

    argv[argc] = NULL;

    return argc;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: find_common_prefix                                               */
/* 功能描述: 在参数字符串列表中,找到这些字符串相同前缀字符的长度            */
/* 输入参数: argv[] --- 指向参数字符串列表的指针                             */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int find_common_prefix(char *const argv[])
{
    int   i;
    int   len;
    char *anchor;
    char *s;
    char *t;

    /**************************************************************************/
    /* 输入参数检查                                                           */
    /**************************************************************************/
    if (*argv == NULL)
    {
        return 0;
    }

    /**************************************************************************/
    /* 从第一个参数字符串开始                                                */
    /**************************************************************************/
    anchor = *argv++;
    len = strlen(anchor);

    /**************************************************************************/
    /* 依次和第一个字符串,进行逐个字符的对比,直到找到不一样的字符;每一次对  */
    /* 比结束返回的是这两个字符串前缀相同的字符长度                          */
    /**************************************************************************/
    while ((t = *argv++) != NULL)
    {
        s = anchor;

        for (i = 0; i < len; i++, t++, s++)
        {
            if (*t != *s)
            {
                break;
            }
        }

        len = s - anchor;
    }

    return len;
}

static void print_argv(const char *banner, const char *leader, const char *sep, int linemax, char *const argv[])
{
    int ll = leader != NULL ? strlen(leader) : 0;
    int sl = sep != NULL ? strlen(sep) : 0;
    int len, i;

    if (banner)
    {
        dbug("\n");
        dbug(banner);
    }

    i = linemax;    /* force leader and newline */

    while (*argv != NULL)
    {
        len = strlen(*argv) + sl;

        if (i + len >= linemax)
        {
            dbug("\n");

            if (leader)
            {
                dbug(leader);
            }

            i = ll - sl;
        }
        else if (sep)
        {
            dbug(sep);
        }

        dbug(*argv++);
        i += len;
    }

    dbug("\n");

    return;
}
#endif

#if 0
static int getc_xmodem(void)
{
    if (tstc())
    {
        return (getc());
    }
    else
    {
        return -1;
    }
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: load_serial_ymodem                                               */
/* 功能描述: 通过ymodem协议从串口传输文件,并保存到指定位置                  */
/* 输入参数: offset --- 文件存储区起始地址                                   */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-02-03              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int load_serial_ymodem(unsigned int offset)
{
    signed int   rlen = 0;
    signed int  xRslt = 0;
    const char *delim = NULL;
    T_yModem_STREAM *ps = NULL;
    IAP_PARAM     *iap = NULL;
    unsigned char *pRcvBuf = NULL;
    unsigned int   pack = 4096;
    unsigned int   store_size = 0;
    unsigned int   store_addr = offset;
    struct device *dev = NULL;
    
    unsigned char sha1out[SHA1_LEN];
    SHA1_CTX sha1 = {0};
    unsigned char md5out[MD5_LEN];
    MD5_CTX  md5  = {0};

    /**************************************************************************/
    /* 查找设备句柄                                                           */
    /**************************************************************************/
    dev = Fn_device_find(OUTSIDE_FLASH);
    if (NULL == dev)
    {
        dbug("## Can't find device %s\r\n", OUTSIDE_FLASH);
        return -1;
    }
    
    /**************************************************************************/
    /* 申请内存,接收数据                                                      */
    /**************************************************************************/
    while(pack)
    {
#ifdef  BSP_USE_RT_THREAD
        pRcvBuf = (unsigned char *)rt_malloc(pack);
#else
        pRcvBuf = (unsigned char *)malloc(pack);
#endif
        if(pRcvBuf == NULL)
        {
            pack >>= 1;
        }
        else
        {
            break;
        }
    }

    if(pack == 0) 
    {
        dbug("## Run out of memory for outside flash\r\n");
        return -2;
    }

    /**************************************************************************/
    /* 申请内存,用于YMODEM的传输控制流                                       */
    /**************************************************************************/
#ifdef  BSP_USE_RT_THREAD
    ps = (T_yModem_STREAM *)rt_malloc(sizeof(T_yModem_STREAM));
#else
    ps = (T_yModem_STREAM *)malloc(sizeof(T_yModem_STREAM));
#endif

    if (NULL == ps)
    {
        dbug("## Run out of memory for ymodem stream\r\n");
        goto EXIT_LABEL;
    }

    memset(ps, 0, sizeof(T_yModem_STREAM));
    /**************************************************************************/
    /* 启用YMODEM的传输控制流                                                */
    /**************************************************************************/
    xRslt = yModem_ReceiveOpen(ps);

    if (!xRslt)
    {
        SHA1_Init(&sha1);
        MD5_Init(&md5);
    
        while ((rlen = yModem_ReceiveRead(ps, pRcvBuf, pack)) > 0)
        {
            Fn_device_write(dev, store_addr, pRcvBuf, rlen);

            store_addr += rlen;
            store_size += rlen;

            SHA1_Update(&sha1, pRcvBuf, rlen);
            MD5_Update(&md5, pRcvBuf, rlen);
        }

        SHA1_Final(&sha1, sha1out);
        MD5_Final(&md5, md5out);
    }

    /**************************************************************************/
    /* 终止YMODEM的传输控制流                                                */
    /**************************************************************************/
    yModem_Terminate(ps, 1);

    if (xRslt)
    {
        dbug("## IAP file receive failed, no file input\r\n");
        xRslt = -3;
        goto EXIT_LABEL;
    }

    if (store_size != ps->xFileLength)
    {
        dbug("## IAP file receive incomplete\r\n");
        xRslt = -4;
        goto EXIT_LABEL;
    }

    dbug("## IAP file receive complete %d Bytes\r\n", store_size);
    /**************************************************************************/
    /* 申请内存,用于YMODEM的传输控制流                                       */
    /**************************************************************************/
#ifdef  BSP_USE_RT_THREAD
    iap = (IAP_PARAM *)rt_malloc(sizeof(IAP_PARAM));
#else
    iap = (IAP_PARAM *)malloc(sizeof(IAP_PARAM));
#endif

    if (NULL == iap)
    {
        dbug("## Run out of memory for IAP param\r\n");
        xRslt = -5;
        goto EXIT_LABEL;
    }
 
    delim = strstr(ps->sFileName, "_") + 2;  /* xxx_v100.bin */
    
    iap->IAP_Flag = APP_UPGRADE_FLAG;
    iap->IAP_Len  = store_size;
    iap->IAP_Version = Fn_atoi(&delim);
    strcpy(iap->IAP_Name, ps->sFileName);
    memcpy(iap->IAP_Md5,  md5out,  MD5_LEN);
    memcpy(iap->IAP_Sha1, sha1out, SHA1_LEN);

    COMM_SetUpdateInfo(iap);
    COMM_SystemStateSwitch(SYS_IAP);

EXIT_LABEL:
#ifdef  BSP_USE_RT_THREAD
    if(NULL != ps)       rt_free(ps);
    if(NULL != iap)      rt_free(iap);
    if(NULL != pRcvBuf)  rt_free(pRcvBuf);
#else
    if(NULL != ps)       free(ps);
    if(NULL != iap)      free(iap);
    if(NULL != pRcvBuf)  free(pRcvBuf);
#endif

    return xRslt;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: cmd_task                                                         */
/* 功能描述: 命令行任务的实现                                                 */
/* 输入参数: pTaskData --- 指向任务参数的指针                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int cmd_task(void)
{
    int len;
    int rc = 1;
    int flag = 0;

    len = readline(CONFIG_SYS_PROMPT);

    if (len > 0)
    {
        strcpy(lastcommand, console_buffer);
    }
    else if (len == 0)
    {
        flag |= CMD_FLAG_REPEAT;
    }

    if (len == -1)
    {
        dbug("^C\r\n");
    }
    else
    {
        rc = run_command(lastcommand, flag);
    }

    if (rc <= 0)
    {
        lastcommand[0] = 0;     /* 忽略无效命令或不可重复命令 */
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: readline                                                         */
/* 功能描述: 从输入流读取一行数据(遇到\r/\n即认为是一行结束)                */
/* 输入参数: prompt --- 指向提示符的指针                                     */
/* 输出参数: 无                                                               */
/* 返 回 值: 读取的字符的数量; -1 - break; -2 - timed out                    */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int readline(const char *const prompt)
{
    console_buffer[0] = '\0';
    return readline_to_buffer(prompt, console_buffer, 0);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: readline_to_buffer                                               */
/* 功能描述: 从输入流读取一行字符到指定的缓存中                              */
/* 输入参数: prompt --- 指向提示符的指针                                      */
/*           buffer --- 指向缓存的指针                                        */
/*           timeout --- 超时时间                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: 读取的字符的数量; -1 - break; -2 - timed out                     */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int readline_to_buffer(const char *const prompt, char *buffer, int timeout)
{
    char *p     = buffer;                          /* 指向缓存区的操作指针   */
    char *pHead = p;                               /* 指向缓存区首地址的指针 */
    int   n     = 0;                               /* 缓冲区索引值           */
    int   plen  = 0;                               /* 提示符的长度           */
    int   col   = 0;                               /* 输出列计数值           */
    char  c;                                       /* 读取的字符             */

    /**************************************************************************/
    /* 输出提示符                                                             */
    /**************************************************************************/
    if (prompt)
    {
        plen = strlen(prompt);
        dbug(prompt);
    }

    col = plen;

    /**************************************************************************/
    /* 循环读取字符                                                           */
    /**************************************************************************/
    while (1)
    {
        c = bsp_console_getc();

        switch (c)
        {
        /**********************************************************************/
        /* 回车/换行的处理: 输出回车换行符,返回长度                          */
        /**********************************************************************/
        case '\r':
        case '\n':
            *p = '\0';
            dbug("\r\n");
            return (p - pHead);

        /**********************************************************************/
        /* 空字符的处理: 忽略                                                 */
        /**********************************************************************/
        case '\0':
            continue;

        /**********************************************************************/
        /* ^C(ctrl+c, ETX, 正文结束)的处理实现: 返回-1,结束处理              */
        /**********************************************************************/
        case 0x03:
            pHead[0] = '\0';
            return (-1);

        /**********************************************************************/
        /* ^U(ctrl+u, NAK, 否定应答)的处理实现: 擦除行                       */
        /**********************************************************************/
        case 0x15:
            while (col > plen)
            {
                dbug(erase_seq);
                --col;
            }

            p = pHead;
            n = 0;
            continue;

        /**********************************************************************/
        /* ^W(ctrl+w, ETB, 传输块结束)的处理实现: 擦除字符                   */
        /**********************************************************************/
        case 0x17:
            p = delete_char(pHead, p, &col, &n, plen);

            while ((n > 0) && (*p != ' '))
            {
                p = delete_char(pHead, p, &col, &n, plen);
            }

            continue;

        /**********************************************************************/
        /* ^H(ctrl+h, BS, 退格) 或 DEL的处理实现                              */
        /**********************************************************************/
        case 0x08:
        case 0x7F:
            p = delete_char(pHead, p, &col, &n, plen);
            continue;

        /**********************************************************************/
        /* 普通字符的处理实现                                                 */
        /**********************************************************************/
        default:
            if (n < CONFIG_SYS_CBSIZE - 2)
            {
                if (c == '\t')
                {
                #ifdef CONFIG_AUTO_COMPLETE
                    *p = '\0';

                    if (cmd_auto_complete(prompt, console_buffer, &n, &col))
                    {
                        p = pHead + n;  /* reset */
                        continue;
                    }
                #endif
                    dbug(tab_seq + (col & 07));
                    col += 8 - (col & 07);
                }
                else
                {
                    ++col;
                    dbug("%c", c);
                }

                *p++ = c;
                ++n;
            }
            else
            {
                dbug("%c", '\a');
            }
        }
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: run_command                                                      */
/* 功能描述: 运行命令                                                         */
/* 输入参数: cmd --- 指向命令字符串的指针                                    */
/*           flag --- 标志位                                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: 0 - 执行成功; 非0 - 执行失败                                    */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int run_command(const char *cmd, int flag)
{
    return buildin_run_command(cmd, flag);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: parse_line                                                       */
/* 功能描述: 从命令行中解析命令参数                                          */
/* 输入参数: line --- 指向命令行字符串的指针                                 */
/* 输出参数: argv[] --- 指向命令行参数列表的指针                             */
/* 返 回 值: 参数个数                                                         */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int parse_line(char *line, char *argv[])
{
    int nargs = 0;

#ifdef DEBUG_PARSER
    dbug("parse_line: \"%s\"\r\n", line);
#endif

    while (nargs < CONFIG_SYS_MAXARGS)
    {
        /**********************************************************************/
        /* 跳过所有的空白符                                                  */
        /**********************************************************************/
        while (IS_BLANK(*line))
        {
            ++line;
        }

        /**********************************************************************/
        /* 遇到了字符串的结束符,没有更多的参数项了,返回                   */
        /**********************************************************************/
        if (*line == '\0')
        {
            argv[nargs] = NULL;
#ifdef DEBUG_PARSER
            dbug("parse_line: nargs=%d\r\n", nargs);
#endif
            return (nargs);
        }

        /**********************************************************************/
        /* 保存参数字符串起始地址                                            */
        /**********************************************************************/
        argv[nargs++] = line;

        /**********************************************************************/
        /* 查找参数字符串的结束位置,即空白符或是字符串结束符               */
        /**********************************************************************/
        while (*line && !IS_BLANK(*line))
        {
            ++line;
        }

        /**********************************************************************/
        /* 遇到了字符串的结束符,没有更多的参数项了,返回                   */
        /**********************************************************************/
        if (*line == '\0')
        {
            argv[nargs] = NULL;
#ifdef DEBUG_PARSER
            dbug("parse_line: nargs=%d\r\n", nargs);
#endif
            return (nargs);
        }

        /**********************************************************************/
        /* 当前的参数字符串,以字符串结束符结尾                              */
        /**********************************************************************/
        *line++ = '\0';
    }

    dbug("Too many args(max %d)\r\n", CONFIG_SYS_MAXARGS);

#ifdef DEBUG_PARSER
    dbug("parse_line: nargs=%d\r\n", nargs);
#endif

    return (nargs);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: cmd_process                                                      */
/* 功能描述: 命令处理的实现                                                   */
/* 输入参数: flag --- 标志位                                                  */
/*           argc --- 参数个数                                                */
/*           argv[] --- 指向参数列表的指针                                    */
/* 输出参数: repeatable --- 该命令是否支持重复执行                           */
/* 返 回 值: 0(CMD_RET_SUCCESS) - 成功                                        */
/*           1(CMD_RET_FAILURE) - 失败                                        */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
E_CMD_RET cmd_process(int flag, int argc, char *const argv[], int *repeatable)
{
    E_CMD_RET rc = CMD_RET_SUCCESS;
    T_CMD_TBL *cmdtp;

    /**************************************************************************/
    /* 在命令行列表中查找命令项                                               */
    /**************************************************************************/
    cmdtp = find_cmd(argv[0]);

    if (cmdtp == NULL)
    {
        dbug("Unknown command '%s', try 'help'\r\n", argv[0]);
        return CMD_RET_FAILURE;
    }

    /**************************************************************************/
    /* 检查参数个数                                                           */
    /**************************************************************************/
    if (argc > cmdtp->maxargs)
    {
        rc = CMD_RET_USAGE;
    }

    /**************************************************************************/
    /* 指向命令行                                                             */
    /**************************************************************************/
    if (!rc)
    {
        rc = (E_CMD_RET)cmd_call(cmdtp, flag, argc, argv);
        *repeatable &= cmdtp->repeatable;
    }

    if (rc == CMD_RET_USAGE)
    {
        rc = (E_CMD_RET)cmd_usage(cmdtp);
    }

    return rc;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: find_cmd                                                         */
/* 功能描述: 查找命令                                                         */
/* 输入参数: cmd --- 指向命令字符串的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: T_CMD_TBL                                                        */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
T_CMD_TBL *find_cmd(const char *cmd)
{
    int len;

    len = (int)&command$$Length;
    dbug("command$$Length = %x\r\n", len);
    return find_cmd_tbl(cmd, (T_CMD_TBL *)&command$$Base, len);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: find_cmd_tbl                                                     */
/* 功能描述: 从指定的命令行列表中查找命令列表项                              */
/* 输入参数: cmd --- 执向命令字符串的指针                                    */
/*           table --- 指向命令列表的指针                                     */
/*           table_len --- 命令行列表长度                                     */
/* 输出参数: 无                                                               */
/* 返 回 值: T_CMD_TBL                                                        */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
T_CMD_TBL *find_cmd_tbl(const char *cmd, T_CMD_TBL *table, int table_len)
{
    T_CMD_TBL  *cmdtp;
    T_CMD_TBL  *cmdtp_temp = table;  /* Init value */
    const char *p;
    int         len;
    int         n_found    = 0;

    if (!cmd)
    {
        return NULL;
    }

    /**************************************************************************/
    /* 有些命令允许使用长度修饰符(如"cp.b"). 只比较命令名直到第一个点.      */
    /**************************************************************************/
    len = ((p = strchr(cmd, '.')) == NULL) ? strlen(cmd) : (p - cmd);

    for (cmdtp = table; cmdtp != table + table_len / sizeof(T_CMD_TBL); cmdtp++)
    {
        if (strncmp(cmd, cmdtp->name, len) == 0)
        {
            if (len == strlen(cmdtp->name))
            {
                return cmdtp;   /* full match */
            }

            cmdtp_temp = cmdtp; /* abbreviated command ? */
            n_found++;
        }
    }

    if (n_found == 1)
    {
        return cmdtp_temp;
    }

    return NULL;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: cmd_usage                                                        */
/* 功能描述: 输出简短的命令使用提示信息                                       */
/* 输入参数: cmdtp --- 指向命令表项的指针                                     */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int cmd_usage(const T_CMD_TBL *cmdtp)
{
    dbug("%s - %s\r\n", cmdtp->name, cmdtp->usage);
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: cmd_help                                                         */
/* 功能描述: help命令的实现                                                   */
/* 输入参数: cmdtp --- 指向命令项描述符的指针                                 */
/*           flag --- 标志位                                                  */
/*           argc --- 参数个数                                                */
/*           argv[] --- 指向参数列表的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int cmd_help(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[])
{
    int i;
    int rcode = 0;
    T_CMD_TBL *ptCmd = NULL;

    /**************************************************************************/
    /* 列出全部命令                                                           */
    /**************************************************************************/
    if (argc == 1)
    {
        for (i = 0; i < (int)(&command$$Length) / sizeof(T_CMD_TBL); i++)
        {
            ptCmd = (T_CMD_TBL *)((char *)&command$$Base + i * sizeof(T_CMD_TBL));
            dbug("%-8s - %s\r\n", ptCmd->name, ptCmd->usage);
        }

        return 0;
    }

    /**************************************************************************/
    /* 列出指定命令的帮助信息                                                 */
    /**************************************************************************/
    for (i = 1; i < argc; ++i)
    {
        if (NULL != (ptCmd = find_cmd_tbl(argv[i],
                                          (T_CMD_TBL *)&command$$Base,
                                          (int)&command$$Length)))
        {
            rcode |= cmd_usage(ptCmd);
        }
        else
        {
            dbug("Unknown command '%s' - try 'help'"
                    " without arguments for list of all"
                    " known commands\r\n", argv[i]);
            rcode = 1;
        }
    }

    return rcode;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: cmd_get_data_width                                               */
/* 功能描述: 检查尺寸规格 .b, .w or .l                                        */
/* 输入参数: arg --- 指向参数字符串的指针                                    */
/*           default_size --- size                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int cmd_get_data_width(char *arg, int default_size)
{
    int len = strlen(arg);

    if (len > 2 && arg[len - 2] == '.')
    {
        switch (arg[len - 1])
        {
        case 'b':
            return 1;

        case 'w':
            return 2;

        case 'l':
            return 4;

        case 's':
            return -2;

        default:
            return -1;
        }
    }

    return default_size;
}

#ifdef CONFIG_AUTO_COMPLETE
int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp)
{
    int n = *np;                            /* 缓存区的长度信息 */
    int col = *colp;                        /* 缓存区的列信息   */
    char *argv[CONFIG_SYS_MAXARGS + 1];     /* NULL 结束        */
    char *cmdv[20];
    char *s;
    char *t;
    const char *sep;
    int i, j, k, len, seplen, argc;
    int cnt;
    char last_char;

    /**************************************************************************/
    /* 检查提示符是否和系统提示符一致                                        */
    /**************************************************************************/
    if (strcmp(prompt, CONFIG_SYS_PROMPT) != 0)
    {
        return 0;
    }

    /**************************************************************************/
    /* 确定缓存区的最后一个字符是什么                                        */
    /**************************************************************************/
    cnt = strlen(buf);

    if (cnt >= 1)
    {
        last_char = buf[cnt - 1];
    }
    else
    {
        last_char = '\0';
    }

    /**************************************************************************/
    /* 复制到将受影响的辅助缓冲区                                            */
    /**************************************************************************/
    strcpy(copy_buffer, buf);

    /**************************************************************************/
    /* 分离为 argv                                                            */
    /**************************************************************************/
    argc = make_argv(copy_buffer, sizeof(argv) / sizeof(argv[0]), argv);

    /**************************************************************************/
    /* 完成并返回可能的完成                                                  */
    /**************************************************************************/
    i = complete_cmdv(argc, argv, last_char, sizeof(cmdv) / sizeof(cmdv[0]), cmdv);

    /* no match; bell and out */
    if (i == 0)
    {
        if (argc > 1)   /* 允许非命令选项卡 */
        {
            return 0;
        }

        putc('\a');
        return 1;
    }

    s   = NULL;
    len = 0;
    sep = NULL;
    seplen = 0;

    if (i == 1)         /* one match; perfect */
    {
        k = strlen(argv[argc - 1]);
        s = cmdv[0] + k;
        len = strlen(s);
        sep = " ";
        seplen = 1;
    }
    else if (i > 1 && (j = find_common_prefix(cmdv)) != 0)    /* more */
    {
        k = strlen(argv[argc - 1]);
        j -= k;

        if (j > 0)
        {
            s = cmdv[0] + k;
            len = j;
        }
    }

    if (s != NULL)
    {
        k = len + seplen;
        /* make sure it fits */
        if (n + k >= CONFIG_SYS_CBSIZE - 2)
        {
            putc('\a');
            return 1;
        }

        t = buf + cnt;

        for (i = 0; i < len; i++)
        {
            *t++ = *s++;
        }

        if (sep != NULL)
        {
            for (i = 0; i < seplen; i++)
            {
                *t++ = sep[i];
            }
        }

        *t = '\0';
        n += k;
        col += k;

        dbug(t - k);

        if (sep == NULL)
        {
            putc('\a');
        }

        *np = n;
        *colp = col;
    }
    else
    {
        print_argv(NULL, "  ", " ", 78, cmdv);
        dbug(prompt);
        dbug(buf);
    }

    return 1;
}
#endif

/*****************************************************************************/
/* Reset命令实现                                                             */
/*****************************************************************************/
int do_reset(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[])
{
    int rcode = 0;

    switch (argc)
    {
    /**********************************************************************/
    /* 复位整个系统                                                       */
    /**********************************************************************/
    case 1:
        dbug("reset board now...\r\n");
        NVIC_SystemReset();
        break;

    /**********************************************************************/
    /* 复位指定的模块                                                     */
    /**********************************************************************/
    case 2:
        if (0 == strcmp(argv[1], "mcu"))
        {
            dbug("reset mcu now...\r\n");
        }
        else if (0 == strcmp(argv[1], "sdev"))
        {
            dbug("reset sdev now...\r\n");
        }
        else
        {
            dbug("no %s module\r\n", argv[1]);
        }
        break;

    default:
        rcode = CMD_RET_USAGE;
        break;
    }

    return rcode;
}
USER_CMD(reset, 2, 1, do_reset, "reset system or module");

/******************************************************************************/
/* Jump命令实现                                                               */
/******************************************************************************/
int do_jump(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[])
{
    int rcode = 0;

    switch (argc)
    {
    /**********************************************************************/
    /* 程序将跳转                                                         */
    /**********************************************************************/
    case 1:
        dbug("boot run will jump ...\r\n");
        COMM_SystemStateSwitch(SYS_LOAD);
        break;

    /**********************************************************************/
    /* nothing                                                            */
    /**********************************************************************/
    case 2:
        break;

    default:
        rcode = CMD_RET_USAGE;
        break;
    }

    return rcode;
}
USER_CMD(jump, 2, 1, do_jump, "jump code to app");

/******************************************************************************/
/* Load命令实现                                                               */
/******************************************************************************/
int do_load_serial_bin(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[])
{
    unsigned long offset = 0;
    int rcode = 0;

    offset = CONFIG_SYS_LOAD_ADDR;

    if (argc >= 2)
    {
        offset = Fn_StrToul(argv[1], NULL, 16);
    }

    if (strcmp(argv[0], "loady") == 0)
    {
        dbug("## Ready for binary (ymodem) download to w25q64 in 0x%08lX...\r\n", offset);
        load_serial_ymodem(offset);
    }

    return rcode;
}
USER_CMD(loady, 2, 0, do_load_serial_bin, "load binary file over serial line(ymodem)");

