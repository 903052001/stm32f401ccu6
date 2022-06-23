/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_rsa.c                                                         */
/* 内容摘要: RSA加密源文件                                                    */
/* 其它说明: 非对称加密                                                       */
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
#include "Fn_rsa.h"

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
static void mbedtls_ctr_drbg_init(mbedtls_ctr_drbg_context *ctx);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static mbedtls_entropy_context ginEntropy;
static mbedtls_ctr_drbg_context ginCtrDrbg;
static mbedtls_rsa_context ginRsa;

/******************************************************************************/
/* 默认的RSA公钥                                                              */
/******************************************************************************/
static const unsigned char g_RSAkey[128] =
{
    0xC6, 0x69, 0xA3, 0xC2, 0x79, 0x06, 0x18, 0xAE,
    0x82, 0xDF, 0xEF, 0x07, 0x6E, 0xA6, 0x9F, 0x9D,
    0xA2, 0x10, 0xE5, 0x0D, 0xE6, 0xF0, 0x11, 0x3D,
    0xBF, 0x7D, 0x95, 0x3B, 0xB1, 0x2A, 0xAB, 0x4B,
    0xA7, 0xC4, 0xAB, 0xAC, 0xDD, 0x1A, 0x23, 0xA2,
    0x01, 0x13, 0x7F, 0x52, 0x68, 0x54, 0xB4, 0x28,
    0xEE, 0x52, 0x41, 0x66, 0xE1, 0x0C, 0xD3, 0xFB,
    0x2C, 0xC9, 0x79, 0xF1, 0x7C, 0x6D, 0x27, 0x1D,
    0x2A, 0x17, 0xDC, 0x8D, 0xE4, 0x77, 0xD0, 0xE3,
    0x1C, 0xB3, 0x37, 0xCB, 0x31, 0x7B, 0x5B, 0xEA,
    0xF9, 0x1E, 0x06, 0xB8, 0x4E, 0x25, 0x0F, 0xDA,
    0xDD, 0xB1, 0xD8, 0xD7, 0xAC, 0xFA, 0x1E, 0x7C,
    0xDD, 0x51, 0x80, 0x30, 0xA4, 0x04, 0x15, 0x9B,
    0x10, 0x3E, 0xBA, 0x4A, 0xA6, 0x08, 0xC7, 0x49,
    0x37, 0xCF, 0xF1, 0x60, 0x53, 0x72, 0x13, 0xC7,
    0xB5, 0x36, 0xFA, 0x7D, 0x27, 0xDB, 0x0F, 0x55,
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
static void mbedtls_ctr_drbg_init(mbedtls_ctr_drbg_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_ctr_drbg_context));
    return;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_RSAEncrypt                                                    */
/* 功能描述: RSA加密算法实现                                                  */
/* 输入参数: key --- 密钥头                                                   */
/*           keyLen --- 密钥长                                                */
/*           input --- 待加密数据头                                           */
/*           inputLen --- 待加密数据长                                        */
/*           out --- 输出密文头                                               */
/*           outLen --- 输出密文长                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_RSAEncrypt(const unsigned char *key, int keyLen, const unsigned char *input, int inputLen, unsigned char *out, int outLen)
{
    int i, ret;
    const char *pers = "mbedtls_pk_encrypt";

    mbedtls_ctr_drbg_init(&ginCtrDrbg);
    mbedtls_entropy_init(&ginEntropy);

    if (0 != (ret = mbedtls_ctr_drbg_seed(&ginCtrDrbg, mbedtls_entropy_func, &ginEntropy,
                                          (const u8 *) pers,
                                          Ql_strlen(pers))))
    {
        SG_LOG_ERROR("mbedtls_ctr_drbg_seed returned -0x%04x", -ret);
        return ret;
    }

    mbedtls_rsa_init(&ginRsa, MBEDTLS_RSA_PKCS_V15, 0);
    ginRsa.len = keyLen;
    mbedtls_mpi_read_string(&ginRsa.N, 16, key);
    mbedtls_mpi_read_string(&ginRsa.E, 16, SG_RSA_E);

    if (mbedtls_rsa_check_pubkey(&ginRsa) != 0)
    {
        SG_LOG_WARN("mbedtls_rsa_check_pubkey error");
    }

    ret = mbedtls_rsa_pkcs1_encrypt(&ginRsa, mbedtls_ctr_drbg_random, &ginCtrDrbg, MBEDTLS_RSA_PUBLIC, inputLen,
                                    input, out);
    if (ret != 0)
    {
        SG_LOG_ERROR("mbedtls_rsa_pkcs1_encrypt error[-0x%04x]", ret);
    }

    mbedtls_ctr_drbg_free(&ginCtrDrbg);
    mbedtls_entropy_free(&ginEntropy);
    mbedtls_rsa_free(&ginRsa);
    return RSA_LEN;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: Fn_RSADecrypt                                                    */
/* 功能描述: RSA解密算法实现                                                  */
/* 输入参数: key --- 密钥头                                                   */
/*           keyLen --- 密钥长                                                */
/*           input --- 待解密数据头                                           */
/*           inputLen --- 待解密数据长                                        */
/*           out --- 输出明文头                                               */
/*           outLen --- 输出明文长                                            */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-10-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int Fn_RSADecrypt(const unsigned char *key, int keyLen, const unsigned char *input, int inputLen, unsigned char *out, int outLen)
{
    int i, ret;
    size_t olen = 0;
    const char *pers = "mbedtls_pk_decrypt";

    mbedtls_ctr_drbg_init(&ginCtrDrbg);
    mbedtls_entropy_init(&ginEntropy);
    if (0 != (ret = mbedtls_ctr_drbg_seed(&ginCtrDrbg, mbedtls_entropy_func, &ginEntropy,
                                          (const u8 *) pers,
                                          Ql_strlen(pers))))
    {
        SG_LOG_ERROR("mbedtls_ctr_drbg_seed returned -0x%04x", -ret);
        return ret;
    }

    mbedtls_rsa_init(&ginRsa, MBEDTLS_RSA_PKCS_V15, 0);
    ginRsa.len = keyLen;
    mbedtls_mpi_read_string(&ginRsa.N, 16, key);
    mbedtls_mpi_read_string(&ginRsa.E, 16, SG_RSA_E);

    if (mbedtls_rsa_check_pubkey(&ginRsa) != 0)
    {
        SG_LOG_WARN("mbedtls_rsa_check_pubkey error");
    }
    ret = mbedtls_rsa_pkcs1_decrypt(&ginRsa, mbedtls_ctr_drbg_random,
                                    &ginCtrDrbg, MBEDTLS_RSA_PUBLIC,
                                    &olen, input, out, outLen);
    if (ret != 0)
    {
        SG_LOG_ERROR("mbedtls_rsa_pkcs1_decrypt error[-0x%04x]", ret);
    }

    mbedtls_ctr_drbg_free(&ginCtrDrbg);
    mbedtls_entropy_free(&ginEntropy);
    mbedtls_rsa_free(&ginRsa);
    return olen;
}

void sg_rsa_check(void)
{
    unsigned char plait[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    unsigned char encrypt[RSA_LEN + 1];
    sg_rsa_encrypt((const unsigned char *)SG_RSA_N, RSA_LEN, encrypt, 8, encrypt, RSA_LEN + 1);
}





