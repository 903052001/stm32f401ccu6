/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_sha1.c                                                        */
/* 内容摘要: SHA-1校验实现源文件                                             */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-10-30                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-10-30        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "Fn_sha1.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/


/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static unsigned int rotl32(unsigned int x, int b);
static unsigned int get32(const void *p);
static unsigned int fun(int t, unsigned int b, unsigned int c, unsigned int d);
static void processBlock(SHA1_CTX *ctx);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
static unsigned int rotl32(unsigned int x, int b)
{
    return (x << b) | (x >> (32 - b));
}

static unsigned int get32(const void *p)      /* 0x1A2B3C4D <-> 0x4D3C2B1A */
{
    const unsigned char *x = (const unsigned char *)p;
    return (x[0] << 24) | (x[1] << 16) | (x[2] << 8) | x[3];
}

static unsigned int fun(int t, unsigned int b, unsigned int c, unsigned int d)
{
    if (!(0 <= t && t < 80))  
    {
        while(1);
    }

    if (t < 20)
        return (b & c) | ((~b) & d);
    else if (t < 40)
        return b ^ c ^ d;
    else if (t < 60)
        return (b & c) | (b & d) | (c & d);
    else if (t < 80)
        return b ^ c ^ d;
    else
        return 0xFFFFFFFF;
}

static void processBlock(SHA1_CTX *ctx)
{
    static const unsigned int k[4] =
    {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };

      signed int t = 0;
    unsigned int w[16];
    unsigned int a = ctx->h[0];
    unsigned int b = ctx->h[1];
    unsigned int c = ctx->h[2];
    unsigned int d = ctx->h[3];
    unsigned int e = ctx->h[4];

    for (t = 0; t < 16; t++)
    {
        w[t] = get32(&((unsigned int *)ctx->block)[t]);
    }

    for (t = 0; t < 80; t++)
    {
        int s = t & 0xf;
        unsigned int temp;
        
        if (t >= 16)
        {
            w[s] = rotl32(w[(s + 13) & 0xf] ^ w[(s + 8) & 0xf] ^ w[(s + 2) & 0xf] ^ w[s], 1);
        }

        temp = rotl32(a, 5) + fun(t, b, c, d) + e + w[s] + k[t / 20];

        e = d;
        d = c;
        c = rotl32(b, 30);
        b = a;
        a = temp;
    }

    ctx->h[0] += a;
    ctx->h[1] += b;
    ctx->h[2] += c;
    ctx->h[3] += d;
    ctx->h[4] += e;

    return;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: SHA1_Init                                                        */
/* 功能描述: SHA1初始化                                                       */
/* 输入参数: sha1 --- SHA1数据结构体指针                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-02-02              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int SHA1_Init(SHA1_CTX *sha1)
{
    sha1->h[0] = 0x67452301;
    sha1->h[1] = 0xefcdab89;
    sha1->h[2] = 0x98badcfe;
    sha1->h[3] = 0x10325476;
    sha1->h[4] = 0xc3d2e1f0;
    sha1->bytes = 0;
    sha1->cur = 0;

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: SHA1_Update                                                      */
/* 功能描述: SHA1更新计算数据包                                               */
/* 输入参数: sha1 --- SHA1数据结构体指针                                      */
/*           input --- 数据包指针                                             */
/*           inlen --- 数据包长度                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int SHA1_Update(SHA1_CTX *sha1, unsigned char *input, unsigned int inlen)
{
    sha1->bytes += inlen;

    while (inlen--)
    {
        /* could optimize the first and last few bytes, and then copy 128 bit blocks with SIMD in between */
        sha1->block[sha1->cur++] = *input++;

        if (sha1->cur == 64)
        {
            processBlock(sha1);
            sha1->cur = 0;
        }
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: SHA1_Final                                                       */
/* 功能描述: SHA1最终计算结果                                                 */
/* 输入参数: sha1 --- SHA1数据结构体指针                                      */
/*           out[SHA1_LEN] --- SHA1计算结果输出区                             */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int SHA1_Final(SHA1_CTX *sha1, unsigned char out[SHA1_LEN])
{
    unsigned char i;
    unsigned long long bits = 0;

    /* 附加分隔符 */
    sha1->block[sha1->cur++] = 0x80;

    if (sha1->cur > 56)
    {
        /* 若块中没有用于64位消息长度的空间则刷新 */
        memset(&sha1->block[sha1->cur], 0, 64 - sha1->cur);
        processBlock(sha1);
        sha1->cur = 0;
    }

    memset(&sha1->block[sha1->cur], 0, 56 - sha1->cur);
    bits = sha1->bytes * 8;

    sha1->block[56] = (unsigned char)(bits >> 56 & 0xFF);
    sha1->block[57] = (unsigned char)(bits >> 48 & 0xFF);
    sha1->block[58] = (unsigned char)(bits >> 40 & 0xFF);
    sha1->block[59] = (unsigned char)(bits >> 32 & 0xFF);
    sha1->block[60] = (unsigned char)(bits >> 24 & 0xFF);
    sha1->block[61] = (unsigned char)(bits >> 16 & 0xFF);
    sha1->block[62] = (unsigned char)(bits >> 8  & 0xFF);
    sha1->block[63] = (unsigned char)(bits >> 0  & 0xFF);
    processBlock(sha1);

    for (i = 0; i < 5; i++)
    {
        *out++ = sha1->h[i] >> 24 & 0xFF;
        *out++ = sha1->h[i] >> 16 & 0xFF;
        *out++ = sha1->h[i] >> 8  & 0xFF;
        *out++ = sha1->h[i] >> 0  & 0xFF;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: SHA1_Test                                                        */
/* 功能描述: 测试用例                                                         */
/* 输入参数: input --- 待校验数据头                                           */
/*           inlen --- 待校验数据长,任意值                                    */
/*           output[SHA1_LEN] --- SHA1校验值输出缓存区                        */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: SHA1的输出是20B,显示的是hex[20]->str[40]或者中间的一部分        */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int SHA1_Test(unsigned char *input, unsigned int inlen, unsigned char output[SHA1_LEN])
{
    SHA1_CTX sha1;

    SHA1_Init(&sha1);
    SHA1_Update(&sha1, input, inlen);  /* while(pack--) */
    SHA1_Final(&sha1, output);         /* 所有分包都Update后统一Final */

    return 0;
}

