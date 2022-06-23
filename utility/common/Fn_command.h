/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_command.h                                                     */
/* 内容摘要: 命令行相关的头文件                                              */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-20                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-20        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_COMMAND_H
#define FN_COMMAND_H

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_md5.h"
#include "Fn_sha1.h"

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
//#define CONFIG_AUTO_COMPLETE                      /* 暂不支持指令自动补全   */
  
#define CMD_FLAG_REPEAT             0x0001          /* 重复上一个命令         */
#define CMD_FLAG_BOOT               0x0002          /* 命令来自boot           */

#define CONFIG_SYS_MAXARGS          6               /* 命令行的最大参数个数   */

#define STRUCT_SECTION              __attribute__((section("command")))

#ifdef CONFIG_AUTO_COMPLETE
#define CMD_COMPLETE(X)             X,
#else
#define CMD_COMPLETE(X)
#endif

#define CMD_MK_ELE_CMPLT(NAME, MAX_ARGS, REP, CMD, USAGE, COMP)                \
                                    {#NAME, MAX_ARGS, REP, CMD, USAGE,         \
                                     CMD_COMPLETE(COMP)}

#define CMD_MK_ELE(NAME, MAX_ARGS, REP, CMD, USAGE)                            \
                                    CMD_MK_ELE_CMPLT(NAME, MAX_ARGS, REP, CMD, \
                                                     USAGE, NULL)

#define USER_CMD_CMPLT(NAME, MAX_ARGS, REP, CMD, USAGE, COMP)                  \
                                    T_CMD_TBL __cmd_##NAME STRUCT_SECTION =    \
                                    CMD_MK_ELE_CMPLT(NAME, MAX_ARGS, REP, CMD, \
                                                     USAGE, COMP)

#define USER_CMD(NAME, MAX_ARGS, REP, CMD, USAGE)                              \
                                    USER_CMD_CMPLT(NAME, MAX_ARGS, REP, CMD,   \
                                                   USAGE, NULL)

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/
/*<STRUCT+>********************************************************************/
/* 结构: T_CMD_TBL                                                            */
/* 注释: 命令表项结构定义                                                    */
/*<STRUCT->********************************************************************/
typedef struct t_cmd_tbl
{
    char *name;                     /* Command Name                           */
    int   maxargs;                  /* maximum number of arguments            */
    int   repeatable;               /* autorepeat allowed?                    */
    int (*cmd)(struct t_cmd_tbl *,  /* Implementation function                */
               int,
               int,
               char *const[]);
    char *usage;                    /* Usage message    (short)               */

#ifdef CONFIG_AUTO_COMPLETE
    int (*complete)(int argc,       /* do auto completion on the arguments    */
                    char *const argv[],
                    char last_char,
                    int maxv,
                    char *cmdv[]);
#endif

} T_CMD_TBL;

/*<ENUM+>**********************************************************************/
/* 枚举: E_CMD_RET                                                            */
/* 注释: 命令执行返回值枚举值                                                */
/*<ENUM->**********************************************************************/
typedef enum e_cmd_ret
{
    CMD_RET_SUCCESS = 0,              /* 0 = Success                          */
    CMD_RET_FAILURE = 1,              /* 1 = Failure                          */
    CMD_RET_USAGE   = -1,             /* Failure, please report 'usage' error */

} E_CMD_RET;

/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
int cmd_task(void);
int readline(const char *const prompt);
int readline_to_buffer(const char *const prompt, char *buffer, int timeout);
int run_command(const char *cmd, int flag);
int parse_line(char *line, char *argv[]);

E_CMD_RET cmd_process(int flag, int argc, char *const argv[], int *repeatable);
T_CMD_TBL *find_cmd(const char *cmd);
T_CMD_TBL *find_cmd_tbl(const char *cmd, T_CMD_TBL *table, int table_len);

int cmd_usage(const T_CMD_TBL *cmdtp);
int cmd_help(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[]);
int cmd_get_data_width(char *arg, int default_size);

int do_load_serial_bin(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[]);
int do_reset(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[]);
int do_jump(T_CMD_TBL *cmdtp, int flag, int argc, char *const argv[]);

#ifdef CONFIG_AUTO_COMPLETE
int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp);
#endif


#ifdef __cplusplus
    }
#endif

#endif


