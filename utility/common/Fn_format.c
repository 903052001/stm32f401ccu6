/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_format.c                                                      */
/* 内容摘要: 数据格式转换源文件                                               */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-04-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-04-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <string.h>
#include "Fn_mctype.h"
#include "Fn_format.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  STR_MAXLEN         (16)          /* 字符串最大长度                  */

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/


/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/


/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: checkCPUendian                                                   */
/* 功能描述: 处理器端序检测                                                   */
/* 输入参数: 无                                                               */
/* 输出参数: 本机大端返回1，小端返回0                                        */
/* 返 回 值: unsigned char                                                    */
/* 操作流程:                                                                  */
/* 其它说明: 单片机为小端序，网络数据和片外flash存储为大端序                */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
unsigned char checkCPUendian(void)
{
    union
    {
        unsigned int  i;
        unsigned char s[4];
    } c;

    c.i = 0x12345678;

    return (0x12 == c.s[0]);
}

/* 本机字节序转网络字节序 */
unsigned int t_htonl(unsigned int h)
{
    return checkCPUendian() ? h : BigLittleSwap32(h);
}

/* 网络字节序转本机字节序 */
unsigned int t_ntohl(unsigned int n)
{
    return checkCPUendian() ? n : BigLittleSwap32(n);
}

/* 本机字节序转网络字节序 */
unsigned short t_htons(unsigned short h)
{
    return checkCPUendian() ? h : BigLittleSwap16(h);
}

/* 网络字节序转本机字节序 */
unsigned short t_ntohs(unsigned short n)
{
    return checkCPUendian() ? n : BigLittleSwap16(n);
}

/******************************************************************************/
/* 将字符转换为小写                                                          */
/******************************************************************************/
char _tolower(char c)
{
    return ((c >= 'A') && (c <= 'Z')) ? ((c - 'A') + 'a') : c;
}

/******************************************************************************/
/* 将字符转换为大写                                                          */
/******************************************************************************/
char _toupper(char c)
{
    return ((c >= 'a') && (c <= 'z')) ? ((c - 'a') + 'A') : c;
}

/******************************************************************************/
/* 转换单个字符转为数值                                                      */
/******************************************************************************/
int _tohex(char c)
{
    int ret = 0;

    if ((c >= '0') && (c <= '9'))
    {
        ret = (c - '0');
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        ret = (c - 'a' + 10);
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        ret = (c - 'A' + 10);
    }
    else
    {
        ret = 16;
    }

    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_ValToStr_Simple                                               */
/* 功能描述: 将数值转换为字符串格式                                          */
/* 输入参数: val --- 数值                                                     */
/* 输出参数: 无                                                               */
/* 返 回 值: char*  转换后的字符串地址                                       */
/* 操作流程:                                                                  */
/* 其它说明: 10进制数的字符串表示, 0x64 = 100 = "100"(已补'\0')              */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
char *Fn_ValToStr_Simple(int val)
{
    static char num_str[11];      /* u32的最大值是10位10进制数 */
    int  pos = 10; 

    num_str[10] = 0;

    if (val == 0)
    {
        return "0";
    }

    while ((val != 0) && (pos > 0))
    {
        num_str[--pos] = (val % 10) + '0';
        val /= 10;
    }

    return &num_str[pos];
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_ValToStr_Strict                                               */
/* 功能描述: 将整型数值,以指定进制格式输出到缓冲区中                        */
/* 输入参数: buf --- 指向缓冲区开始处的指针                                  */
/*           end --- 指向缓冲区结束处的指针                                  */
/*           num --- 整型数值                                                 */
/*           base --- 进制                                                    */
/*           size --- 整型数占用输出缓冲的最小宽度                           */
/*           type --- 格式类型(按位使用)                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: char *                                                           */
/* 操作流程: 0x400(16) = 1024(10) -> "400"(16) = "1024(10)"                   */
/* 其它说明: buf[size]由符号,数据,补位3部分构成                              */
/*           左对齐为符号+数据+补位; 右对齐为符号+补位+数据                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
char *Fn_ValToStr_Strict(char *buf, char *end, unsigned long long num, int base, int size, int type)
{
    char c;
    char sign;
    char tmp[66];  /* 64位的ull数加上一字节符号位('+'/'-')以及'\0'共计66字节 */
    int  i = 0;
    static const char digits[16] = "0123456789abcdef";

    /**************************************************************************/
    /* 如果是左对齐，则取消补'0'的操作，即左对齐在后面只能补' '             */
    /**************************************************************************/
    if (type & LEFT)
    {
        type &= ~ZEROPAD;
    }

    /**************************************************************************/
    /* 确定填充字符                                                           */
    /**************************************************************************/
    c = (type & ZEROPAD) ? '0' : ' ';

    /**************************************************************************/
    /* 确定需要打印的符号字符                                                */
    /**************************************************************************/
    sign = 0;

    if (type & SIGN)
    {
        if ((signed long long)num < 0)
        {
            sign = '-';
            num = - (signed long long)num;
            size--;
        }
        else if (type & PLUS)
        {
            sign = '+';
            size--;
        }
    }

    /**************************************************************************/
    /* 将数值转换为字符串(逆序)                                               */
    /**************************************************************************/
    if (num == 0)
    {
        tmp[i++] = '0';
    }
    else
    {
        while (num != 0)
        {
            tmp[i++] = digits[num % base];
            num = num / base;
        }
    }

    size -= i;   /* 补位的长度 */

    /**************************************************************************/
    /* 填充符号                                                               */
    /**************************************************************************/
    if (sign)
    {
        if (buf <= end)
        {
            *buf = sign;
        }

        ++buf;
    }

    /**************************************************************************/
    /* 不是左对齐则采用选择的字符进行填充,即右对齐可在前面补' '或'0'        */
    /**************************************************************************/
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf <= end)
            {
                *buf = c;
            }

            ++buf;
        }
    }

    /**************************************************************************/
    /* 填充数值转换后的字符(顺序)                                            */
    /**************************************************************************/
    while (i-- > 0)
    {
        if (buf <= end)
        {
            *buf = tmp[i];
        }

        ++buf;
    }

    /**************************************************************************/
    /* 余下的空间以空格填充(如果是右对齐则size等于0)                         */
    /**************************************************************************/
    while (size-- > 0)
    {
        if (buf <= end)
        {
            *buf = ' ';
        }

        ++buf;
    }

    return buf;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_HexToChar                                                     */
/* 功能描述: 一个16进制数转一个字符                                          */
/* 输入参数: hex --- 16进制数值                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: char                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 0 ~ 9 ~ 10 ~ 15 ~ 16... -> '0' ~ '9' ~ 'a/A' ~ 'f/F' ~ '.'...    */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-07-13              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
char Fn_HexToChar(unsigned char hex)
{
    char ch;

#ifdef SHOW_WITH_UPPER
    ch = (hex <= 9) ? ('0' + hex) : ((hex <= 15) ? ('A' + hex - 10) : '.');
#else
    ch = (hex <= 9) ? ('0' + hex) : ((hex <= 15) ? ('a' + hex - 10) : '.');
#endif

    return ch;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_CharToHex                                                     */
/* 功能描述: 一个字符转一个16进制数                                          */
/* 输入参数: ch --- 待转字符                                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: unsigned char                                                    */
/* 操作流程:                                                                  */
/* 其它说明: '0' ~ '9' ~ 'a/A' ~ 'f/F' -> 0 ~ 9 ~ 10 ~ 15  other -> 16        */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-07-13              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
unsigned char Fn_CharToHex(char ch)
{
    return _tohex(ch) & 0xFF;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_AsciiToHex                                                    */
/* 功能描述: 将字符串转换为HEX格式数(一个字符转一个HEX格式数)                */
/* 输入参数: pStr --- 字符串指针                                              */
/*           Len_Str --- 字符串长(由strlen取)                                 */
/*           pHex --- 存HEX数指针                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: "9ABCD" -> [9, 10, 11, 12, 13]                                   */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_AsciiToHex(unsigned char *pStr, int Len_Str, unsigned char *pHex)
{
    while (Len_Str--)
    {
        *pHex++ = _tohex(*pStr++);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_StrToHex                                                      */
/* 功能描述: 将字符串转换为HEX格式数据(2个字符转一个HEX格式数)               */
/* 输入参数: pStr --- 字符串指针                                              */
/*           lStr --- 字符串长度,由strlen取,不计入'\0'                        */
/*           pHex --- 输出HEX数据区指针                                       */
/* 输出参数: 转换后的HEX数据长度                                              */
/* 返 回 值: int                                                              */
/* 操作流程: "ABCD" -> 0xAB 0xCD                                              */
/* 其它说明: 字符串不只由字符0~9和A/a~F/f组成,会出现其他字符'@''#''%'        */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_StrToHex(const char *pStr, int lStr, unsigned char *pHex)
{
    int  i;
    unsigned char h1, h2;
    
    if ((NULL == pStr) || (0 == lStr) || (NULL == pHex))
    {
        return -1;
    }
    
    for (i = 0; i < (lStr >> 1); i++)
    {
        h1 = Fn_CharToHex(pStr[(i << 1) + 0]); 
        h2 = Fn_CharToHex(pStr[(i << 1) + 1]);

        if ((h1 >= 16) || (h2 >= 16))
        {
            return -2;
        }
       
        pHex[i] = (h1 << 4) | h2;
    }

    return i;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_HexToStr                                                      */
/* 功能描述: 将HEX格式数组转换为字符串                                       */
/* 输入参数: pHex --- 待转数据区指针                                          */
/*           lHex --- HEX格式数据长(由sizeof取)                               */
/*           pStr --- 输出字符串指针(申请缓存区长度至少为(lHex * 2 + 1)      */
/* 输出参数: 转换后的字符串数据长度(由strlen()取,不计'\0')                   */
/* 返 回 值: int                                                              */
/* 操作流程: 0xAB 0xCD -> "ABCD"    hex格式数据取值为[0x00-0xFF]              */
/* 其它说明: 注意字符串结尾\0,防止越界,多申请1B,手动置为0                    */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_HexToStr(const unsigned char *pHex, int lHex, char *pStr)
{
    int  i;
    
    if ((NULL == pHex) || (0 == lHex) || (NULL == pStr))
    {
        return -1;
    }

    for (i = 0; i < lHex; i++)
    {
        *pStr++ = Fn_HexToChar(pHex[i] >> 4);
        *pStr++ = Fn_HexToChar(pHex[i] & 0xF);
    }

    *pStr = '\0';

    return i << 1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_BCDToStr                                                      */
/* 功能描述: BCD码转字符串                                                    */
/* 输入参数: pBCD --- BCD码指针                                               */
/*           Len_BCD --- BCD码长度                                            */
/*           pStr --- 字符串指针(字符串长度为Len_BCD<<1)                     */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 0x2A = 42 = 42(BCD) = 0100 0010(BCD) = 0010 1010(b)              */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              Leonard       v1.0.0        创建函数           */
/*<FUNC->**********************************************************************/
void Fn_BCDToStr(unsigned char *pBCD, int Len_BCD, unsigned char *pStr)
{
    unsigned char ch = 0;
    unsigned char cnts;

    for (cnts = 0; cnts < Len_BCD; cnts++)
    {
        ch = *pBCD >> 4;
        *pStr++ = "0123456789ABCDEF"[ch];

        ch = *pBCD & 0x0F;
        *pStr++ = "0123456789ABCDEF"[ch];

        pBCD++;
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_StrToBCD                                                      */
/* 功能描述: 字符串码转BCD码                                                  */
/* 输入参数: pStr --- 字符串指针                                              */
/*           strLen --- 字符串长度                                            */
/*           bcd --- BCD码指针                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-04-19              Leonard       v1.0.0        创建函数           */
/*<FUNC->**********************************************************************/
int Fn_StrToBCD(unsigned char *pStr, int strLen, unsigned char *bcd)
{
    int byteLens  = 0;
    int byteVal   = 0;
    int index     = 0;
    int cnts      = 0;
    int odd       = 0;
    unsigned char  ascTbl[6] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    if ((NULL == pStr) || (0 == strLen))
    {
        goto EXIT_LABEL;
    }

    /*************************************************************************/
    /* 判断字符串的长度是否为奇数,并计算转换的字节最小长度                 */
    /*************************************************************************/
    odd = strLen & 0x01;
    byteLens = strLen >> 1;

    /*************************************************************************/
    /* 逐字节进行转换                                                        */
    /*************************************************************************/
    for (cnts = 0; cnts < byteLens; cnts++)
    {
        /*********************************************************************/
        /* 转换高4位                                                         */
        /*********************************************************************/
        if ((*pStr >= '0') && (*pStr <= '9'))
        {
            byteVal = (*pStr) << 4;
        }
        else if ((*pStr >= 'A') && (*pStr <= 'F'))
        {
            index = *pStr - 'A';
            byteVal = ascTbl[index] << 4;
        }
        else if ((*pStr >= 'a') && (*pStr <= 'f'))
        {
            index = *pStr - 'a';
            byteVal = ascTbl[index] << 4;
        }
        else
        {
            byteVal = 0xF0;
        }

        pStr++;

        /*********************************************************************/
        /* 转换低4位                                                         */
        /*********************************************************************/
        if ((*pStr >= '0') && (*pStr <= '9'))
        {
            byteVal |= ((*pStr) & 0x0F);
        }
        else if ((*pStr >= 'A') && (*pStr <= 'F'))
        {
            index = *pStr - 'A';
            byteVal |= ascTbl[index];
        }
        else if ((*pStr >= 'a') && (*pStr <= 'f'))
        {
            index = *pStr - 'a';
            byteVal |= ascTbl[index];
        }
        else
        {
            byteVal = 0x0F;
        }

        /*********************************************************************/
        /* 偏移到下一个字节的转换                                           */
        /*********************************************************************/
        *bcd++ = byteVal;
        pStr++;
    }

    /*************************************************************************/
    /* 需要补低4位                                                           */
    /*************************************************************************/
    if (0 != odd)
    {
        if ((*pStr >= '0') && (*pStr <= '9'))
        {
            byteVal = (*pStr) << 4;
        }
        else if ((*pStr >= 'A') && (*pStr <= 'F'))
        {
            index = *pStr - 'A';
            byteVal = ascTbl[index] << 4;
        }
        else if ((*pStr >= 'a') && (*pStr <= 'f'))
        {
            index = *pStr - 'a';
            byteVal = ascTbl[index] << 4;
        }
        else
        {
            byteVal = 0xF0;
        }

        byteVal |= 0x0F;
        *bcd = byteVal;

        byteLens++;
    }

EXIT_LABEL:
    return byteLens;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_StrToul                                                       */
/* 功能描述: 将指定的字符串转换为无符号的长整型数                            */
/* 输入参数: cp   --- 指向要转换的字符串的指针(必须有结束符'\0')             */
/*           base --- 转换进制，自动识别8进制和16进制; 0-默认10进制          */
/* 输出参数: endp --- 指向转换结束位置的指针                                 */
/* 返 回 值: 转换后得到的无符号整数值                                        */
/* 操作流程: 11(10) = 11; 17(10) = 0x11; 9(10) = 011; (8进制用0表示而非o)    */
/* 其它说明: 16进制识别时用的是"0x"/"0X"                                     */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
unsigned long Fn_StrToul(const char *cp, char **endp, unsigned int base)
{
    unsigned long result = 0;
    unsigned long value;

    if (*cp == '0')
    {
        cp++;

        if (((*cp == 'X') || (*cp == 'x')) && isxdigit(cp[1]))
        {
            base = 16;
            cp++;
        }

        if (!base)
        {
            base = 8;
        }
    }

    if (!base)
    {
        base = 10;
    }

    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : (islower(*cp) ? _toupper(*cp) : *cp) - 'A' + 10) < base)
    {
        result = result * base + value;
        cp++;
    }

    if (endp)
    {
        *endp = (char *)cp;
    }

    return result;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_StrToul_Strict                                                */
/* 功能描述: 带有严格格式检查的字符串转为无符号长整型值的实现               */
/* 输入参数: cp --- 要转换的字符串                                           */
/*           base --- 转换进制                                                */
/* 输出参数: res  --- 指向转换后的数值的指针                                 */
/* 返 回 值: 0-成功；其他-失败                                               */
/* 操作流程:                                                                  */
/* 其它说明: 要转换的字符串cp须全部由数字字符组成，或是最后一个为换行字符.  */
/*           例如: "1234567\0" 或 "1234567\n\0"                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_StrToul_Strict(const char *cp, unsigned int base, unsigned long *res)
{
    char *tail;
    unsigned long val;
    int len;

    *res = 0;
    len  = strlen(cp);

    if (len == 0)
    {
        return -1;
    }

    val = Fn_StrToul(cp, &tail, base);

    if (tail == cp)
    {
        return -2;
    }

    if ((*tail == '\0') || ((len == (int)(tail - cp) + 1) && (*tail == '\n')))
    {
        *res = val;
        return 0;
    }

    return -3;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_StrTosl                                                       */
/* 功能描述: 将字符串转换为有符号长整型数                                    */
/* 输入参数: cp --- 指向要转换的字符串的指针                                 */
/*           *endp --- 指向转换完成后结束位置的指针                          */
/*           base --- 转换进制                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: long                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
long Fn_StrTosl(const char *cp, char **endp, unsigned int base)
{
    if (*cp == '-')
    {
        return -Fn_StrToul(cp + 1, endp, base);
    }

    return Fn_StrToul(cp, endp, base);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_itoa                                                          */
/* 功能描述: 将整数转换为字符串                                              */
/* 输入参数: val --- 需要转换的整数值                                        */
/*           str --- 指向字符串的指针                                         */
/*           radix --- val的进制                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: 转换后的字符串的长度(不计入'\0')                                */
/* 操作流程:                                                                  */
/* 其它说明: 100 -> "100"(10) -> "64"(16)                                     */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_itoa(int val, char *str, int radix)
{
    char *p;
    int   minus;
    char  buf[STR_MAXLEN];
    int   maxLens;
    int   lens = 0;
    int   cnts;

    p = &buf[STR_MAXLEN];
    *--p = '\0';
    lens++;

    if (val < 0)
    {
        minus = 1;
        val = -val;
        maxLens = STR_MAXLEN - 2;
    }
    else
    {
        minus = 0;
        maxLens = STR_MAXLEN - 1;
    }

    if (0 == val)
    {
        *--p = '0';
        lens++;
    }
    else
    {
        while (val > 0)
        {
          //*--p = "0123456789abcdef"[val % radix];
            *--p = "0123456789ABCDEF"[val % radix];
            val /= radix;

            if (++lens >= maxLens)
            {
                break;
            }
        }
    }

    if (0 != minus)
    {
        *--p = '-';
        lens++;
    }

    for (cnts = 0; cnts < lens; cnts++)
    {
        str[cnts] = p[cnts];
    }

    return (lens - 1);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_atoi                                                          */
/* 功能描述: 将字符串转化为10进制数值,同时移动指针到转换结束位置            */
/* 输入参数: str --- 指向字符串指针的指针                                    */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 其它说明: &"0x64" -> 100 or &"100" -> 100                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_atoi(const char **str)
{
    int  val = 0;
    int  flag = 1;
    int  first = 1;
    int  digit = 0;
    int  radix = 10;
    
    /**************************************************************************/
    /* 跳过空格                                                               */
    /**************************************************************************/
    while (**str == ' ')
    {
        (*str)++;
    }

    /**************************************************************************/
    /* 第一个字符若是'-'，说明可能是负数                                     */
    /**************************************************************************/
    if (**str == '-')
    {
        flag = 0;
        (*str)++;
    }
    /**************************************************************************/
    /* 第一个字符若是'+'，说明可能是正数                                     */
    /**************************************************************************/
    else if (**str == '+')
    {
        flag = 1;
        (*str)++;
    }

    while(**str)
    {
        if (first && ((*str)[0] == '0') && (((*str)[1] == 'x') || ((*str)[1] == 'X')))
        {
            radix = 16;
            *str += 2;
        }
        
        first = 0;
        digit = _tohex(**str);
 
        if ((isxdigit(**str)) && (digit < radix))
        {
            val = (val * radix) + digit;
            (*str)++;
        }
        else
        {
            break;
        }
    }

    if (flag == 0)
    {
        val = -val;
    }

    return val;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_atoi_version                                                  */
/* 功能描述: 从指定的字符串中解析出数字(用于版本号解析)                     */
/* 输入参数: str --- 指向字符串的指针                                        */
/*           mark --- 分隔符                                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: "1.0.0" -> 100 or "1 0 0" -> 100                                 */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_atoi_version(const char *str, char mark)
{
    int val = 0;

    while (*str == ' ')
    {
        str++;
    }

    while (*str)
    {
        if (isdigit(*str))
        {
            val = (val * 10) + _tohex(*str++);
        }
        else if (*str++ == mark)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return val;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_itoa_version                                                  */
/* 功能描述: 从指定的字符串中解析出数字(用于版本号解析)                     */
/* 输入参数: val --- 10进制下待转数值                                        */
/*           str --- 指向字符串的指针                                        */
/*           mark --- 分隔符                                                  */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 100 -> "1.0.0" or 100 -> "1 0 0"                                 */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_itoa_version(int val, char *str, char mark)
{
    char *p;
    int   minus;
    char  buf[STR_MAXLEN];
    int   maxLens;
    int   lens = 0;
    int   cnts;
    
    p = &buf[STR_MAXLEN];
    *--p = '\0';
    lens++;

    if (val < 0)
    {
        minus = 1;
        val = -val;
        maxLens = STR_MAXLEN - 2;
    }
    else
    {
        minus = 0;
        maxLens = STR_MAXLEN - 1;
    }

    if (0 == val)
    {
        *--p = '0';
        lens++;
    }
    else
    {   
        for (cnts = 0; cnts < 3; cnts++)
        {
            *--p = "0123456789"[val % 10];
            *--p = mark;
            val /= 10;
            lens += 2;
        }

        ++p; --lens;
        
        while (val > 0)
        {
            *--p = "0123456789"[val % 10];
            val /= 10;

            if (++lens >= maxLens)
            {
                break;
            }
        }
    }

    if (0 != minus)
    {
        *--p = '-';
        lens++;
    }

    for (cnts = 0; cnts < lens; cnts++)
    {
        str[cnts] = p[cnts];
    }

    return (lens - 1);
}


