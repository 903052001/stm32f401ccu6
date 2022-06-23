/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_xxtea.c                                                       */
/* 内容摘要: XxTEA加密源文件                                                  */
/* 其它说明: 对称加密                                                         */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-10-29                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-10-29        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/


/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Fn_xxtea.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define dbug                printf

#define MX                 ((z>>5^y<<2) + (y>>3^z<<4)^(sum^y) + (k[p&3^e]^z))
#define DELTA              (0x9E3779B9)

#define LOOPTIMES                1                           /* 轮询加密次数 */
#define ENCRYPT_INT_BUFF        256

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int  bytes2int(unsigned char *bytes);
static void int2bytes(int data, unsigned char *bytes);
static void TEA_EncryptCore(int block_size, int *buf, int *key);
static void TEA_DecrpytCore(int block_size, int *buf, int *key);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static int sg_key_buff[4] = { 0 };                         /* key区 定长16B  */
static int sg_data_buff[ENCRYPT_INT_BUFF] = { 0 };         /* 加解密的输出区 */

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bytes2int                                                        */
/* 功能描述: 字节转换为字                                                     */
/* 输入参数: bytes --- 指向字节流的指针                                       */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int bytes2int(unsigned char *bytes)
{
    int result = 0;

    result |= ((bytes[0] << 0)  & 0xFF);
    result |= ((bytes[1] << 8)  & 0xFF00);
    result |= ((bytes[2] << 16) & 0xFF0000);
    result |= ((bytes[3] << 24) & 0xFF000000);

    return result;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: int2bytes                                                        */
/* 功能描述: 字转换为字节                                                     */
/* 输入参数: data --- 要转换的字                                              */
/*           bytes --- 指向字节缓冲区的指针                                   */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void int2bytes(int data, unsigned char *bytes)
{
    bytes[0] = (unsigned char)((0xFF & data) >> 0);
    bytes[1] = (unsigned char)((0xFF00 & data) >> 8);
    bytes[2] = (unsigned char)((0xFF0000 & data) >> 16);
    bytes[3] = (unsigned char)((0xFF000000 & data) >> 24);
    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: TEA_EncryptCore                                                  */
/* 功能描述: XxTEA加密核心算法                                                */
/* 输入参数: block_size --- 块大小                                            */
/*           buf --- 指向加密数据的指针                                       */
/*           key --- 指向秘钥的指针                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void TEA_EncryptCore(int block_size, int *buf, int *key)
{
    char n   = (block_size / 4);
    int *v   = (int *)buf;
    int *k   = (int *)key;
    long z   = v[n - 1];
    long y   = v[0];
    long sum = 0;
    long e;
    char p;
    char q;

    q = LOOPTIMES + 52 / n;

    while (q-- > 0)
    {
        sum += DELTA;
        e = sum >> 2 & 3;

        for (p = 0; p < (n - 1); p++)
        {
            y = v[p + 1];
            z = v[p] += MX;
        }
        y = v[0];
        z = v[n - 1] += MX;
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: TEA_DecrpytCore                                                  */
/* 功能描述: XxTEA解密核心函数                                                */
/* 输入参数: block_size --- 块大小                                            */
/*           buf --- 指向解密数据的指针                                       */
/*           key --- 指向密钥的指针                                           */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void TEA_DecrpytCore(int block_size, int *buf, int *key)
{
    char  n   = (block_size / 4);
    long *v   = (long *)buf;
    long *k   = (long *)key;
    long  z   = v[n - 1];
    long  y   = v[0];
    long  sum = 0;
    long  e;
    unsigned char p;
    unsigned char q;

    q = LOOPTIMES + 52 / n;
    sum = q * DELTA;

    while (sum != 0)
    {
        e = sum >> 2 & 3;

        for (p = (n - 1); p > 0; p--)
        {
            z = v[p - 1];
            y = v[p] -= MX;
        }
        z = v[n - 1];
        y = v[0] -= MX;
        sum -= DELTA;
    }

    return;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_XxTEASetKey                                                   */
/* 功能描述: 设置XxTEA算法的密钥                                             */
/* 输入参数: key --- 指向秘钥的指针                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程: 修改key[16]后调用此函数                                         */
/* 其它说明: 密钥定长16B即128位                                              */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_XxTEASetKey(unsigned char *key)
{
    for (int i = 0; i < 4; i++)
    {
        sg_key_buff[i] = bytes2int(&key[i * 4]);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_XxTEAEncrypt                                                  */
/* 功能描述: XxTEA加密算法实现                                                */
/* 输入参数: in --- 待加密数据头                                              */
/*           inlen --- 待加密数据长(<= 1018B)                                 */
/*           out --- 输出密文头                                               */
/* 输出参数: 输出密文长(<= 1024 且被4整除,而非向上取整2^n)                   */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_XxTEAEncrypt(unsigned char *in, int inlen, unsigned char *out)
{
    int i, block_size = ((inlen + 2) / 4 + 1) * 4;
    unsigned char *copy = NULL;

    if (inlen > 1018) return -1;

    copy = (unsigned char *)malloc(block_size);
    if (NULL == copy) return -1;

    memset(sg_data_buff, 0, ENCRYPT_INT_BUFF * sizeof(int));
    memcpy(copy, in, inlen);

    if (block_size > inlen)
    {
        memset(&copy[inlen], 0, block_size - inlen);
    }

    for (i = inlen + 1; i >= 2; i--)
    {
        copy[i] = copy[i - 2];
    }

    copy[0] = (inlen >> 8) & 0xFF;
    copy[1] = (inlen >> 0) & 0xFF;

    for (i = 0; i < block_size / 4; i++)
    {
        sg_data_buff[i] = bytes2int(&copy[i * 4]);
    }

    free(copy);
    TEA_EncryptCore(block_size, sg_data_buff, sg_key_buff);

    for (i = 0; i < block_size / 4; i++)
    {
        int2bytes(sg_data_buff[i], &out[i * 4]);
    }

    return block_size;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_XxTEADecrypt                                                  */
/* 功能描述: XxTEA解密算法实现                                                */
/* 输入参数: in --- 待解密数据头                                              */
/*           inlen --- 待解密数据长(<= 1024 且被4整除)                        */
/*           out --- 输出明文头                                               */
/* 输出参数: 输出明文长                                                       */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_XxTEADecrypt(unsigned char *in, int inlen, unsigned char *out)
{
    int i, outlen = 0;
    unsigned char *copy = NULL;

    if ((inlen > 1024) || ((inlen % 4) != 0)) return -1;

    copy = (unsigned char *)malloc(inlen);
    if (NULL == copy) return -1;

    memset(sg_data_buff, 0, ENCRYPT_INT_BUFF * sizeof(int));

    for (i = 0; i < inlen / 4; i++)
    {
        sg_data_buff[i] = bytes2int(&in[i * 4]);
    }

    TEA_DecrpytCore(inlen, sg_data_buff, sg_key_buff);

    for (i = 0; i < inlen / 4; i++)
    {
        int2bytes(sg_data_buff[i], &copy[i * 4]);
    }

    outlen = ((int)copy[0] << 8) | (int)copy[1];

    if (outlen > inlen)
    {
        return 0;
    }

    for (i = 0; i < outlen; i++)
    {
        out[i] = copy[i + 2];
    }

    free(copy);
    return outlen;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_XxTEATest                                                     */
/* 功能描述: XxTEA算法测试                                                    */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void Fn_XxTEATest(void)
{
    int len;
    unsigned char key[16] = {"0123456789ABCDEF"};
    unsigned char input[20] = {"ABCDE"};
    unsigned char output[20] = { 0 };

    Fn_XxTEASetKey(key);

    len = Fn_XxTEAEncrypt(input, 5, output);
    dbug("\r\nencode-> ");
    for (int i = 0; i < len; i++)
    {
        dbug("%#x ", output[i]);
    }

    dbug("\r\ndecode-> ");
    len = Fn_XxTEADecrypt(output, len, output);
    for (int i = 0; i < len; i++)
    {
        dbug("%#x ", output[i]);
    }

    return;
}


