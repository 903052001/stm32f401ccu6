/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.              */
/*                                                                            */
/* 文件名称: Fn_aes.c                                                         */
/* 内容摘要: AES加密算法源文件                                               */
/* 其它说明: 支持ECB+CBC,无填充模式                                          */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-07-14                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-07-14        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "Fn_aes.h"

/******************************************************************************/
/*                                外部引用声明                               */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define dbug    printf

#define AES_ECB
//#define AES_CTR
//#define AES_CBC

/******************************************************************************/
/*                              内部数据类型定义                             */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static void AES_SubBytes(unsigned char state[], const unsigned char sbox[]);
static void AES_AddRoundKey(unsigned char state[], const unsigned char rkey[]);
static void AES_ShiftRows(unsigned char state[], const unsigned char shifttab[]);
static void AES_MixColumns(unsigned char state[]);
static void AES_MixColumns_Inv(unsigned char state[]);
static int  AES_ExpandKey(unsigned char output[], unsigned char key[], int keyLen);
static void AES_16B_Encrypt(unsigned char block[], unsigned char key[], int keyLen);
static void AES_16B_Decrypt(unsigned char block[], unsigned char key[], int keyLen);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static const unsigned char AES_Sbox[256] =
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};

static const unsigned char AES_RSbox[256] =
{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
};

static const unsigned char AES_xtime[256] =
{
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E,
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E,
    0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBE,
    0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
    0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE,
    0x1B, 0x19, 0x1F, 0x1D, 0x13, 0x11, 0x17, 0x15, 0x0B, 0x09, 0x0F, 0x0D, 0x03, 0x01, 0x07, 0x05,
    0x3B, 0x39, 0x3F, 0x3D, 0x33, 0x31, 0x37, 0x35, 0x2B, 0x29, 0x2F, 0x2D, 0x23, 0x21, 0x27, 0x25,
    0x5B, 0x59, 0x5F, 0x5D, 0x53, 0x51, 0x57, 0x55, 0x4B, 0x49, 0x4F, 0x4D, 0x43, 0x41, 0x47, 0x45,
    0x7B, 0x79, 0x7F, 0x7D, 0x73, 0x71, 0x77, 0x75, 0x6B, 0x69, 0x6F, 0x6D, 0x63, 0x61, 0x67, 0x65,
    0x9B, 0x99, 0x9F, 0x9D, 0x93, 0x91, 0x97, 0x95, 0x8B, 0x89, 0x8F, 0x8D, 0x83, 0x81, 0x87, 0x85,
    0xBB, 0xB9, 0xBF, 0xBD, 0xB3, 0xB1, 0xB7, 0xB5, 0xAB, 0xA9, 0xAF, 0xAD, 0xA3, 0xA1, 0xA7, 0xA5,
    0xDB, 0xD9, 0xDF, 0xDD, 0xD3, 0xD1, 0xD7, 0xD5, 0xCB, 0xC9, 0xCF, 0xCD, 0xC3, 0xC1, 0xC7, 0xC5,
    0xFB, 0xF9, 0xFF, 0xFD, 0xF3, 0xF1, 0xF7, 0xF5, 0xEB, 0xE9, 0xEF, 0xED, 0xE3, 0xE1, 0xE7, 0xE5,
};

static const unsigned char AES_ShiftRowTab[16] =
{
    0x00, 0x05, 0x0A, 0x0F, 0x04, 0x09, 0x0E, 0x03, 0x08, 0x0D, 0x02, 0x07, 0x0C, 0x01, 0x06, 0x0B,
};

static const unsigned char AES_ShiftRowTab_Inv[16] =
{
    0x00, 0x0D, 0x0A, 0x07, 0x04, 0x01, 0x0E, 0x0B, 0x08, 0x05, 0x02, 0x0F, 0x0C, 0x09, 0x06, 0x03,
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
static void AES_SubBytes(unsigned char state[], const unsigned char sbox[])
{
    for (int i = 0; i < 16; i++)
    {
        state[i] = sbox[state[i]];
    }

    return;
}

static void AES_AddRoundKey(unsigned char state[], const unsigned char rkey[])
{
    for (int i = 0; i < 16; i++)
    {
        state[i] ^= rkey[i];
    }

    return;
}

static void AES_ShiftRows(unsigned char state[], const unsigned char shifttab[])
{
    unsigned char h[16];
    memcpy(h, state, 16);

    for (int i = 0; i < 16; i++)
    {
        state[i] = h[shifttab[i]];
    }

    return;
}

static void AES_MixColumns(unsigned char state[])
{
    for (int i = 0; i < 16; i += 4)
    {
        unsigned char s0 = state[i + 0], s1 = state[i + 1];
        unsigned char s2 = state[i + 2], s3 = state[i + 3];
        unsigned char h  = s0 ^ s1 ^ s2 ^ s3;

        state[i + 0] ^= h ^ AES_xtime[s0 ^ s1];
        state[i + 1] ^= h ^ AES_xtime[s1 ^ s2];
        state[i + 2] ^= h ^ AES_xtime[s2 ^ s3];
        state[i + 3] ^= h ^ AES_xtime[s3 ^ s0];
    }

    return;
}

static void AES_MixColumns_Inv(unsigned char state[])
{
    for (int i = 0; i < 16; i += 4)
    {
        unsigned char s0 = state[i + 0], s1 = state[i + 1];
        unsigned char s2 = state[i + 2], s3 = state[i + 3];
        unsigned char h  = s0 ^ s1 ^ s2 ^ s3;

        unsigned char xh = AES_xtime[h];
        unsigned char h1 = AES_xtime[AES_xtime[xh ^ s0 ^ s2]] ^ h;
        unsigned char h2 = AES_xtime[AES_xtime[xh ^ s1 ^ s3]] ^ h;

        state[i + 0] ^= h1 ^ AES_xtime[s0 ^ s1];
        state[i + 1] ^= h2 ^ AES_xtime[s1 ^ s2];
        state[i + 2] ^= h1 ^ AES_xtime[s2 ^ s3];
        state[i + 3] ^= h2 ^ AES_xtime[s3 ^ s0];
    }

    return;
}

static int AES_ExpandKey(unsigned char output[], unsigned char key[], int keyLen)
{
    int i, j, ks, Rcon = 1;
    unsigned char temp1[4], temp2[4];

    switch (keyLen)
    {
    case 16:
        ks = 16 * (10 + 1);
        break;

    case 24:
        ks = 16 * (12 + 1);
        break;

    case 32:
        ks = 16 * (14 + 1);
        break;

    default:
        dbug("\r\nOnly key lengths of 16, 24 or 32 bytes allowed!\r\n");
        break;
    }

    for (i = 0; i < keyLen; i++)
    {
        output[i] = key[i];
    }

    for (i = keyLen; i < ks; i += 4)
    {
        memcpy(temp1, &output[i - 4], 4);

        if (i % keyLen == 0)
        {
            temp2[0] = AES_Sbox[temp1[1]] ^ Rcon;
            temp2[1] = AES_Sbox[temp1[2]];
            temp2[2] = AES_Sbox[temp1[3]];
            temp2[3] = AES_Sbox[temp1[0]];
            memcpy(temp1, temp2, 4);

            if ((Rcon <<= 1) >= 256)
            {
                Rcon ^= 0x11b;
            }
        }
        else if ((keyLen > 24) && (i % keyLen == 16))
        {
            temp2[0] = AES_Sbox[temp1[0]];
            temp2[1] = AES_Sbox[temp1[1]];
            temp2[2] = AES_Sbox[temp1[2]];
            temp2[3] = AES_Sbox[temp1[3]];
            memcpy(temp1, temp2, 4);
        }

        for (j = 0; j < 4; j++)
        {
            output[i + j] = output[i + j - keyLen] ^ temp1[j];
        }
    }

    return ks;
}

static void AES_16B_Encrypt(unsigned char block[], unsigned char key[], int keyLen)
{
    int i = 0;

    AES_AddRoundKey(block, &key[0]);

    for (i = 16; i < (keyLen - 16); i += 16) /* 单包固定16B */
    {
        AES_SubBytes(block, AES_Sbox);
        AES_ShiftRows(block, AES_ShiftRowTab);
        AES_MixColumns(block);
        AES_AddRoundKey(block, &key[i]);
    }

    AES_SubBytes(block, AES_Sbox);
    AES_ShiftRows(block, AES_ShiftRowTab);
    AES_AddRoundKey(block, &key[i]);

    return;
}

static void AES_16B_Decrypt(unsigned char block[], unsigned char key[], int keyLen)
{
    AES_AddRoundKey(block, &key[keyLen - 16]);
    AES_ShiftRows(block, AES_ShiftRowTab_Inv);
    AES_SubBytes(block, AES_RSbox);

    for (int i = (keyLen - 32); i >= 16; i -= 16)
    {
        AES_AddRoundKey(block, &key[i]);
        AES_MixColumns_Inv(block);
        AES_ShiftRows(block, AES_ShiftRowTab_Inv);
        AES_SubBytes(block, AES_RSbox);
    }

    AES_AddRoundKey(block, &key[0]);

    return;
}

#ifdef AES_CTR
static void IV_cunt(unsigned char out_iV[], unsigned char IV[], int cunt, unsigned char jump)
{
    unsigned char  i, temp[16];
    int add_buf, addcunt = cunt;

    for (i = 0; i < 16; i++)
    {
        temp[15 - i] = Fn_ascii_to_hex(IV[31 - (i * 2 + 1)], IV[31 - (i * 2)]) ;

        if (addcunt != 0)
        {
            add_buf = addcunt * jump + temp[15 - i];
            temp[15 - i] = add_buf % 256;
            addcunt = add_buf / 256;
        }
    }

    for (i = 0; i < 16; i++)
    {
        out_iV[i] = temp[i];
    }

    return;
}
#endif
/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_AESEncrypt                                                    */
/* 功能描述: AES_ECB/CBC/CTR_128/192/256加密实现                              */
/* 输入参数: in[] --- 明文头                                                  */
/*           inlen --- 明文长(任意长)                                         */
/*           key[] --- 密钥头                                                 */
/*           keylen --- 密钥长                                                */
/*           out[] --- 密文头                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int 密文长(必须被16整除,且明文长 <= 密文长)                     */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-07-14              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_AESEncrypt(unsigned char in[], int inlen, unsigned char key[], int keylen, unsigned char out[])
{
    int i, j;
    unsigned char  block[16] = {0};
    int blocks = 0;
    int outlen = 0;
    unsigned char  expandkey[16 * (10 + 1)] = {0};
#ifdef AES_CBC
    unsigned char  IV_key[16] = {0};
#endif

    blocks = ((inlen % 16) != 0) ? (inlen / 16 + 1) : (inlen / 16);

#ifndef AES_CTR
    unsigned char  padding = 0;

    if (inlen % 16)
    {
#ifdef AES_ECB
        padding = 0x00;
#else
        padding = 16 - (inlen % 16);
#endif
    }
    else
    {
#ifdef AES_ECB
        padding = 0x00;
#else
        padding = 0x10;
#endif
    }

    for (j = 0; j < blocks; j++)
    {
        for (i = 0; i < keylen; i++)
        {
            if (i + (j * 16) >= inlen)
            {
                block[i] = padding;
            }
            else
            {
                block[i] = in[i + (j * 16)];
            }
        }

#ifdef AES_CBC
        for (i = 0; i < keylen; i++)
        {
            block[i] ^= IV_key[i];
        }
#endif

        AES_16B_Encrypt(block, expandkey, AES_ExpandKey(expandkey, key, keylen));

        for (i = 0; i < keylen; i++)
        {
            out[outlen++] = block[i];
#ifdef AES_CBC
            IV_key[i] = block[i];
#endif
        }
    }
#else
    unsigned char IV_key[16] = {0};
    int   expandkeyLen = AES_ExpandKey(expandkey, key, keylen);

    for (j = 0; j < blocks; j++)
    {
        IV_cunt(block, IV_key, j, 1);

        AES_16B_Encrypt(block, expandkey, expandkeyLen);

        for (i = 0; i < keylen; i++)
        {
            out[outlen++] = out[outlen] ^ block[i];
        }
    }
#endif

    return outlen;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: AES_Decrypt                                                      */
/* 功能描述: AES_ECB/CBC/CTR_128/192/256解密实现                              */
/* 输入参数: in[] --- 密文头                                                  */
/*           inlen --- 密文长(必须被16整除)                                   */
/*           key[] --- 密钥头                                                 */
/*           keylen --- 密钥长                                                */
/*           out[] --- 明文头                                                 */
/* 输出参数: 无                                                               */
/* 返 回 值: int 明文长(<= 密文长)                                            */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-07-14              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_AESDecrypt(unsigned char in[], int inlen, unsigned char key[], int keylen, unsigned char out[])
{
    int i, j;
    unsigned char  block[16] = {0};
    int blocks = 0;
    int outlen = 0;
    unsigned char  expandkey[16 * (10 + 1)] = {0};

#ifdef AES_CBC
    unsigned char  padding = 0;
    unsigned char  padding_end = 0;
    unsigned char  IV_key[16] = {0};
    unsigned char  IV_temp[16] = {0};
#endif

    if (inlen % 16 != 0)
    {
        dbug("\r\nCiphertext length must be 16 bytes aligned!\r\n");
        return 0;
    }

    blocks = inlen / 16;

#ifndef AES_CTR
    for (j = 0; j < blocks; j++)
    {
        for (i = 0; i < keylen; i++)
        {
            if (i + (j * 16) >= inlen)
            {
                block[i] = 0x00;
            }
            else
            {
                block[i] = in[i + (j * 16)];
            }

#ifdef AES_CBC
            IV_temp[i] = block[i];
#endif
        }

        AES_16B_Decrypt(block, expandkey, AES_ExpandKey(expandkey, key, keylen));

        for (i = 0; i < keylen; i++)
        {
#ifdef AES_CBC
            block[i] ^= IV_key[i];
            IV_key[i] = IV_temp[i];
#endif
            out[outlen++] = block[i];
        }
    }
#else
    unsigned char IV_key[16] = {0};
    int   expandkeyLen = AES_ExpandKey(expandkey, key, keylen);

    for (j = 0; j < blocks; j++)
    {
        IV_cunt(block, IV_key, j, 1);

        AES_16B_Decrypt(block, expandkey, expandkeyLen);

        for (i = 0; i < keylen; i++)
        {
            out[outlen++] = out[outlen] ^ block[i];
        }
    }
#endif

#ifdef AES_CBC
    padding = out[outlen - 1];

    if (padding <= 0x10)
    {
        for (i = 0; i < padding; i++)
        {
            if (out[outlen - 1 - i] != padding)
            {
                padding_end = 1;
            }
        }
    }

    if (padding_end == 0)
    {
        if (padding <= 0x16)
        {
            outlen -= padding;
        }
    }
#endif

    return outlen;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: AES_test                                                         */
/* 功能描述: 测试用例                                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-07-14              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int AES_test(void)
{
    int len;
    unsigned char in3[3]  = {"123"};
    unsigned char key[16] = {"1234567890123456"};
    unsigned char out[16] = {0};

    len = Fn_AESEncrypt(in3, 3, key, 16, out);
    len = Fn_AESDecrypt(out, 16, key, 16, out);
    return len;
}

