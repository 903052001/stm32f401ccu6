/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_format.h                                                      */
/* 内容摘要: 数据格式转换头文件                                               */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-04-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-04-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_FORMAT_H
#define FN_FORMAT_H

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
#define  SHOW_WITH_UPPER                  /* 大写字符串显示                  */
#define  ZEROPAD            (0x01)        /* 用'0'填充(默认' '填充)          */
#define  SIGN               (0x02)        /* 有/无符号长整型数(默认无符号数) */
#define  PLUS               (0x04)        /* 显示'+'号(默认不显示)           */
#define  LEFT               (0x08)        /* 左对齐(默认右对齐)              */

/* 短整型大小端互换 */
#define BigLittleSwap16(A)  ((((unsigned short)(A) & 0xFF00) >> 8)     | \
                             (((unsigned short)(A) & 0x00FF) << 8))

/* 长整型大小端互换 */
#define BigLittleSwap32(A)  ((((unsigned int)(A) & 0xFF000000) >> 24)  | \
                             (((unsigned int)(A) & 0x00FF0000) >> 8)    | \
                             (((unsigned int)(A) & 0x0000FF00) << 8)    | \
                             (((unsigned int)(A) & 0x000000FF) << 24))

/******************************************************************************/
/*                              全局数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/
unsigned char  checkCPUendian(void);
unsigned int   t_ntohl(unsigned int n);
unsigned int   t_htonl(unsigned int h);
unsigned short t_ntohs(unsigned short n);
unsigned short t_htons(unsigned short h);

char _tolower(char c);
char _toupper(char c);
int  _tohex(char c);

char *Fn_ValToStr_Simple(int val);
char *Fn_ValToStr_Strict(char *buf, char *end, unsigned long long num, int base, int size, int type);

char Fn_HexToChar(unsigned char hex);
unsigned char Fn_CharToHex(char ch);
void Fn_AsciiToHex(unsigned char *pStr, int Len_Str, unsigned char *pHex);

int  Fn_StrToHex(const char *pStr, int lStr, unsigned char *pHex);
int  Fn_HexToStr(const unsigned char *pHex, int lHex, char *pStr);

void Fn_BCDToStr(unsigned char *pBCD, int Len_BCD, unsigned char *pStr);
int  Fn_StrToBCD(unsigned char *pStr, int strLen, unsigned char *bcd);

int  Fn_atoi(const char **str);
int  Fn_atoi_version(const char *str, char mark);
int  Fn_itoa(int val, char *str, int radix);
int  Fn_itoa_version(int val, char *str, char mark);

unsigned long Fn_StrToul(const char *cp, char **endp, unsigned int base);
int  Fn_StrToul_Strict(const char *cp, unsigned int base, unsigned long *res);
long Fn_StrTosl(const char *cp, char **endp, unsigned int base);



#ifdef __cplusplus
}
#endif

#endif


