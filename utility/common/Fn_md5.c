/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_md5.c                                                         */
/* 内容摘要: MD5校验实现源文件                                               */
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
#include <stdio.h>
#include <string.h>
#include "Fn_md5.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  F(x,y,z)                       ((x & y) | (~x & z))
#define  G(x,y,z)                       ((x & z) | (y & ~z))
#define  H(x,y,z)                       ((x ^ y ^ z))
#define  I(x,y,z)                       ((y ^ (x | ~z)))
#define  ROTATE_LEFT(x,n)               ((x << n) | (x >> (32 - n)))

#define  FF(a,b,c,d,x,s,ac)              \
         {                               \
             a += F(b,c,d) + x + ac;     \
             a  = ROTATE_LEFT(a,s);      \
             a += b;                     \
         }

#define  GG(a,b,c,d,x,s,ac)              \
         {                               \
             a += G(b,c,d) + x + ac;     \
             a  = ROTATE_LEFT(a,s);      \
             a += b;                     \
         }

#define  HH(a,b,c,d,x,s,ac)              \
         {                               \
             a += H(b,c,d) + x + ac;     \
             a  = ROTATE_LEFT(a,s);      \
             a += b;                     \
         }

#define  II(a,b,c,d,x,s,ac)              \
         {                               \
             a += I(b,c,d) + x + ac;     \
             a  = ROTATE_LEFT(a,s);      \
             a += b;                     \
         }

/******************************************************************************/
/*                              内部数据类型定义                             */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static void MD5_Encode(unsigned char *output, unsigned int *input, unsigned int inlen);
static void MD5_Decode(unsigned int *output, unsigned char *input, unsigned int inlen);
static void MD5_Transform(unsigned int state[4], unsigned char block[64]);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static unsigned char PADDING[64] = { 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/******************************************************************************/
/*                                内部函数实现                               */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Encode                                                       */
/* 功能描述: MD5编码实现                                                      */
/* 输入参数: output --- 密文缓存区                                            */
/*           input --- 明文数据头                                             */
/*           inlen --- 明文数据长                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void MD5_Encode(unsigned char *output, unsigned int *input, unsigned int inlen)
{
    unsigned int i = 0, j = 0;

    while (j < inlen)
    {
        output[j + 0] = (input[i] >> 0)  & 0xFF;
        output[j + 1] = (input[i] >> 8)  & 0xFF;
        output[j + 2] = (input[i] >> 16) & 0xFF;
        output[j + 3] = (input[i] >> 24) & 0xFF;
        
        i += 1;
        j += 4;
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Decode                                                       */
/* 功能描述: MD5解码实现                                                      */
/* 输入参数: output --- 明文缓存区                                            */
/*           input --- 密文数据头                                             */
/*           inlen --- 密文数据长                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void MD5_Decode(unsigned int *output, unsigned char *input, unsigned int inlen)
{
    unsigned int i = 0, j = 0;

    while (j < inlen)
    {
        output[i] = ((unsigned int)input[j + 0] << 0)  |
                    ((unsigned int)input[j + 1] << 8)  |
                    ((unsigned int)input[j + 2] << 16) |
                    ((unsigned int)input[j + 3] << 24);
        i += 1;
        j += 4;
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Transform                                                    */
/* 功能描述: MD5转秩                                                          */
/* 输入参数: state[4] --- key                                                 */
/*           block[64] --- 数据块                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void MD5_Transform(unsigned int state[4], unsigned char block[64])
{
    unsigned int a = state[0];
    unsigned int b = state[1];
    unsigned int c = state[2];
    unsigned int d = state[3];
    unsigned int x[64] = {0};

    MD5_Decode(x, block, 64);
    
    /* Round 1 */
    FF(a, b, c, d, x[ 0],  7, 0xd76aa478); /*  1 */
    FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /*  2 */
    FF(c, d, a, b, x[ 2], 17, 0x242070db); /*  3 */
    FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /*  4 */
    FF(a, b, c, d, x[ 4],  7, 0xf57c0faf); /*  5 */
    FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /*  6 */
    FF(c, d, a, b, x[ 6], 17, 0xa8304613); /*  7 */
    FF(b, c, d, a, x[ 7], 22, 0xfd469501); /*  8 */
    FF(a, b, c, d, x[ 8],  7, 0x698098d8); /*  9 */
    FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */
    FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12],  7, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */
    /* Round 2 */
    GG(a, b, c, d, x[ 1],  5, 0xf61e2562); /* 17 */
    GG(d, a, b, c, x[ 6],  9, 0xc040b340); /* 18 */
    GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */
    GG(a, b, c, d, x[ 5],  5, 0xd62f105d); /* 21 */
    GG(d, a, b, c, x[10],  9,  0x2441453); /* 22 */
    GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */
    GG(a, b, c, d, x[ 9],  5, 0x21e1cde6); /* 25 */
    GG(d, a, b, c, x[14],  9, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */
    GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */
    GG(a, b, c, d, x[13],  5, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[ 2],  9, 0xfcefa3f8); /* 30 */
    GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */
    GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */
    /* Round 3 */
    HH(a, b, c, d, x[ 5],  4, 0xfffa3942); /* 33 */
    HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */
    HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[ 1],  4, 0xa4beea44); /* 37 */
    HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */
    HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */
    HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13],  4, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */
    HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */
    HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */
    HH(a, b, c, d, x[ 9],  4, 0xd9d4d039); /* 45 */
    HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */
    /* Round 4 */
    II(a, b, c, d, x[ 0],  6, 0xf4292244); /* 49 */
    II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */
    II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */
    II(a, b, c, d, x[12],  6, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */
    II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */
    II(a, b, c, d, x[ 8],  6, 0x6fa87e4f); /* 57 */
    II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */
    II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[ 4],  6, 0xf7537e82); /* 61 */
    II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */
    II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    return;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Init                                                         */
/* 功能描述: MD5初始化                                                        */
/* 输入参数: md5 --- MD5数据结构体指针                                        */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int MD5_Init(MD5_CTX *md5)
{
    md5->count[0] = 0;
    md5->count[1] = 0;
    md5->state[0] = 0x67452301;
    md5->state[1] = 0xEFCDAB89;
    md5->state[2] = 0x98BADCFE;
    md5->state[3] = 0x10325476;

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Update                                                       */
/* 功能描述: MD5更新计算数据包                                                */
/* 输入参数: md5 --- MD5数据结构体指针                                        */
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
int MD5_Update(MD5_CTX *md5, unsigned char *input, unsigned int inlen)
{
    unsigned int i = 0;
    unsigned int index = 0; 
    unsigned int partlen = 0;

    index = (md5->count[0] >> 3) & 0x3F;
    partlen = 64 - index;
    md5->count[0] += inlen << 3;
    
    if (md5->count[0] < (inlen << 3))
    {
        md5->count[1]++;
    }
    
    md5->count[1] += inlen >> 29;

    if (inlen >= partlen)
    {
        memcpy(&md5->buffer[index], input, partlen);
        MD5_Transform(md5->state, md5->buffer);

        for (i = partlen; i + 64 <= inlen; i += 64)
        {
            MD5_Transform(md5->state, &input[i]);
        }
        
        index = 0;
    }
    else
    {
        i = 0;
    }
    
    memcpy(&md5->buffer[index], &input[i], inlen - i);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Final                                                        */
/* 功能描述: MD5最终计算结果                                                  */
/* 输入参数: md5 --- MD5数据结构体指针                                        */
/*           out[MD5_LEN] --- MD5计算结果输出区                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int MD5_Final(MD5_CTX *md5, unsigned char out[MD5_LEN])
{
    unsigned int  index = 0, padlen = 0;
    unsigned char bits[8] = {0};

    index = (md5->count[0] >> 3) & 0x3F;
    padlen = (index < 56) ? (56 - index) : (120 - index);
    
    MD5_Encode(bits, md5->count, 8);
    MD5_Update(md5, PADDING, padlen);
    MD5_Update(md5, bits, 8);
    MD5_Encode(out, md5->state, MD5_LEN);

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: MD5_Test                                                         */
/* 功能描述: 测试用例                                                         */
/* 输入参数: input --- 待校验数据头                                           */
/*           inlen --- 待校验数据长,任意值                                    */
/*           output[MD5_LEN] --- MD5校验值输出缓存区                          */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: MD5的输出是16B,显示的是hex[16]->str[32]或者str[8,24)            */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int MD5_Test(unsigned char *input, unsigned int inlen, unsigned char output[MD5_LEN])
{
    MD5_CTX md5;

    MD5_Init(&md5);
    MD5_Update(&md5, input, inlen);  /* while(pack--) */
    MD5_Final(&md5, output);         /* 所有分包都Update后统一Final */

    return 0;
}
