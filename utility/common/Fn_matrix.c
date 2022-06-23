/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_matrix.c                                                      */
/* 内容摘要: 矩阵计算源文件                                                   */
/* 其它说明: 若矩阵A*X=B,则X=A逆*B                                            */
/*   + X + Y  = 2    + 1 1 +   + X +   + 2 +    + X +   + 1 1 +      + 2 +    */
/*   |            => |     | * |   | = |   | => |   | = |     | 逆 * |   |    */
/*   + X + 2Y = 3    + 1 2 +   + Y +   + 3 +    + Y +   + 1 2 +      + 3 +    */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-26                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-26        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/


/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Fn_matrix.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define  dbug   printf

#define  EXPRESS_MAX_LEN    128          /* 单条字符串格式函数表达式最大长度 */
#define  MATRIX_MAX_ORDER   21           /* 最大矩阵阶数，最大未知数个数     */

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static double matrix_inverse_of_order_n(double matrix[], int n);
static int    matrix_multiply_of_order_n(double matrix1[], int row1, int rank1, double matrix2[], int row2, int rank2, double matrix3[]);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: matrix_inverse_of_order_n                                        */
/* 功能描述: 获取一个N阶矩阵的行列式值及其逆矩阵                             */
/* 输入参数: matrix[] --- 输入矩阵指针                                        */
/*           n --- 矩阵阶数(必须是方阵)                                       */
/* 输出参数: N阶矩阵的行列式值                                               */
/* 返 回 值: static double                                                    */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static double matrix_inverse_of_order_n(double matrix[], int n)
{
    if (n > 1)
    {
        int     index1, index2, index3, row, rank, porn;
        double  val = 0.0f, temp = 0.0f, adjoint = 0.0f;
        double *pRemain = malloc((n - 1) * (n - 1) * sizeof(double));
        double *pMatrix = malloc(n * n * sizeof(double));

        if ((NULL == pRemain) || (NULL == pMatrix))
        {
            if (NULL != pMatrix)  free(pMatrix);
            if (NULL != pRemain)  free(pRemain);
            return -1;
        }

        for (row = 0; row < n; row++)
        {
            for (rank = 0; rank < n; rank++)
            {
                for (index3 = 0, index1 = 0; index1 < n; index1++)
                {
                    if (index1 != row)
                    {
                        for (index2 = 0; index2 < n; index2++)
                        {
                            if (index2 != rank)
                            {
                                pRemain[index3++] = matrix[index1 * n + index2];
                            }
                        }
                    }
                }

                temp = matrix_inverse_of_order_n(pRemain, n - 1);
                porn = ((row + 1 + rank + 1) % 2) ? -1 : 1;
                adjoint = porn * temp;
                pMatrix[rank * n + row] = adjoint;
                if (row < 1)  val += adjoint * matrix[row * n + rank];
            }
        }

        for (index1 = 0; index1 < n * n; index1++)
        {
            matrix[index1] = pMatrix[index1] / val;
        }

        if (NULL != pMatrix)  free(pMatrix);
        if (NULL != pRemain)  free(pRemain);

        return val;
    }
    else
        return matrix[0];
}

/*<FUNC+>**********************************************************************/
/* 函数名称: matrix_multiply_of_order_n                                       */
/* 功能描述: 两个矩阵相乘                                                     */
/* 输入参数: matrix1[] --- 矩阵1指针                                          */
/*           row1 --- 矩阵1行数                                               */
/*           rank1 --- 矩阵1列数                                              */
/*           matrix2[] --- 矩阵2指针                                          */
/*           row2 --- 矩阵2行数                                               */
/*           rank2 --- 矩阵2列数                                              */
/*           matrix3[] --- 输出矩阵指针                                       */
/* 输出参数: 输出矩阵大小                                                     */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int matrix_multiply_of_order_n(double matrix1[], int row1, int rank1, double matrix2[], int row2, int rank2, double matrix3[])
{
    int index1, index2, index3;
    double val = 0.0f;

    if (rank1 != row2)  return -1;

    for (index1 = 0; index1 < row1; index1++)
    {
        for (index2 = 0; index2 < rank2; index2++)
        {
            for (val = 0.0f, index3 = 0; index3 < rank1; index3++)
            {
                val += matrix1[index1 * rank1 + index3] * matrix2[index2 + rank2 * index3];
            }
            matrix3[index1 * rank2 + index2] = val;
        }
    }

    return row1 * rank2;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: matrix_solution_of_order_n                                       */
/* 功能描述: 求解矩阵                                                         */
/* 输入参数: *express[] --- 函数表达式指针集(表达式单行长度小于128B)         */
/*           表达式格式: "fF + gG + hH + iI + ... + zZ = @\r\n",空格和\r\n可  */
/*           有可无,其中 f, h, g, i ... z 为系数; F, G, H, I ... Z 为未知数;  */
/*           系数为负数时'+'号必须写为'-'号, 系数为1时必须写1; @为一个常数,  */
/*           unknum --- 未知数个数(求解n各未知数则需要n个表达式)             */
/*           result[] --- 结果输出矩阵                                        */
/* 输出参数: 输出矩阵大小                                                     */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 1.未知数表示符不能用D/d,E/e; 2.未知数表示符从F开始计(必须大写) */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int matrix_solution_of_order_n(char *express[], int unknum, double result[])
{
    int     index1, index2;
    char   *pexp = NULL;
    char    c[2] = {0};
    char    exp[EXPRESS_MAX_LEN] = {0};
    double  A_num = 0.0f, B_num = 0.0f;
    double *pA_matrix = malloc(unknum * unknum * sizeof(double));
    double *pB_matrix = malloc(unknum * 1      * sizeof(double));

    if ((NULL == pA_matrix) || (NULL == pB_matrix))
    {
        if (NULL != pA_matrix)  free(pA_matrix);
        if (NULL != pB_matrix)  free(pB_matrix);
        return -1;
    }

    if (unknum > MATRIX_MAX_ORDER)  return -2;

    /* 提取系数构造矩阵 */
    for (index1 = index2 = 0; index2 < unknum; index2++, index1 = 0)
    {
        pexp = express[index2];
        memset(exp, 0, sizeof(exp));

        do
        {
            if (*pexp != ' ')
            {
                exp[index1++] = *pexp;
            }
        }
        while (*pexp++);

        for (pexp = exp, index1 = 0; index1 < unknum; index1++)
        {
            A_num = atof(pexp);
            pA_matrix[index2 * unknum + index1] = A_num;
            c[0] = 'F' + index1;
            pexp = strstr(exp, c) + 1;
        }

        B_num = atof(strstr(exp, "=") + 1);
        pB_matrix[index2] = B_num;
    }

    /* 矩阵回显 */
    for (int i = 0; i < unknum * unknum; i++)
    {
        if (i % unknum == 0) dbug("\r\nA-> ");
        dbug("%f ", pA_matrix[i]);
    }

    for (int i = 0; i < unknum; i++)
    {
        if (i % unknum == 0) dbug("\r\nB-> ");
        dbug("%f ", pB_matrix[i]);
    }

    /* 求解A逆矩阵 */
    matrix_inverse_of_order_n(pA_matrix, unknum);

    /* A逆矩阵 * B矩阵 */
    index2 = matrix_multiply_of_order_n(pA_matrix, unknum, unknum, pB_matrix, unknum, 1, result);

    /* 打印各未知数的解 */
    for (index1 = 0; index1 < index2; index1++)
    {
        c[0] = 'F' + index1;
        dbug("%c = %f \r\n", c[0], result[index1]);
    }

    if (NULL != pA_matrix)  free(pA_matrix);
    if (NULL != pB_matrix)  free(pB_matrix);

    return unknum;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: import_matrix_express_from_file                                  */
/* 功能描述: 从文件导入矩阵表达式                                            */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 表达式格式: "fF + gG + hH + iI + ... + zZ = @\r\n",必须\r\n结尾 */
/*           每行一个表达式                                                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void import_matrix_express_from_file(void)
{
    char *pfile =
    #include "Fn_matrix.txt"
    ;

    int    index1 = 0, index2 = 0;
    double out[16] = {0};
    char *c = pfile;
    char *pExp = NULL;
    char  c1 = 0, c2 = 0;
    char  exp[EXPRESS_MAX_LEN] = {0};
    char *express[MATRIX_MAX_ORDER] = {0};

    do
    {
        if (index2 >= EXPRESS_MAX_LEN)  goto EXIT_FUN;

        c2 = c1;
        c1 = *c;
        exp[index2++] = c1;

        if ((c2 == '\r') && (c1 == '\n'))
        {
            if (index1 >= MATRIX_MAX_ORDER)  goto EXIT_FUN;

            exp[index2++] = '\0';
            pExp = malloc(index2);

            if (pExp != NULL)
            {
                memcpy(pExp, exp, index2);
                express[index1++] = pExp;
                index2 = 0;
            }
            else
            {
                goto EXIT_FUN;
            }
        }
    }
    while (*c++);

    /* 回显表达式 */
    for (index2 = 0; index2 < index1; index2++)
    {
        dbug("exp = %s", express[index2]);
    }

    matrix_solution_of_order_n(express, index1, out);

EXIT_FUN:
    for (index2 = 0; index2 < index1; index2++)
    {
        if (express[index2] != NULL)  free(express[index2]);
    }

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: range_and_rate_by_two_points                                     */
/* 功能描述: 平面任意两点围成框所占范围和比重                                */
/* 输入参数: box --- 目标框参数指针                                           */
/*           matrix[] --- 输入/输出矩阵                                       */
/*           len --- 输入长度(单位为sizeof(double),而非字节B)                 */
/* 输出参数: 输出矩阵有效长度                                                 */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 落点采用左上原则; x轴向右,y轴向下,类似图片xy轴                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int range_and_rate_by_two_points(box *box, double matrix[], int len)
{
    int index_x, index_y;
    int ltpx = box->ltpx;
    int ltpy = box->ltpy;
    int rbpx = box->rbpx;
    int rbpy = box->rbpy;
    int xLen = box->xLen;
    int yLen = box->yLen;
    int xGap = box->xGap;
    int yGap = box->yGap;
    /* xy轴上的网格数 */
    int xGrids  = (xLen % xGap == 0) ? (xLen / xGap) : (xLen / xGap + 1);
    int yGrids  = (yLen % yGap == 0) ? (yLen / yGap) : (yLen / yGap + 1);
    /* 边角剩余长度 */
    int xRemain = xLen - ((xGrids - 1) * xGap);  // xLen % xGap;  /* 自己体会为何 */
    int yRemain = yLen - ((yGrids - 1) * yGap);  // yLen % yGap;  /* 不使用此方式 */
    /* 目标框范围 */
    int box_x = rbpx - ltpx;
    int box_y = rbpy - ltpy;
    /* 左上、右下点所在网格序号 */
    int _ltpx, _ltpy, _rbpx, _rbpy;
    /* 两点离网格边界距离 */
    int dt_x1, dt_y1, dt_x2, dt_y2;
    /* 跨越网格数 */
    int grid_dt_x, grid_dt_y;

    /* 参数校验 */
    if (xGrids * yGrids > len)           return -1;
    if ((ltpx > rbpx) || (ltpy > rbpy))  return -2;
    if ((xGap > xLen) || (yGap > yLen))  return -3;
    if ((ltpx > xLen) || (ltpy > yLen))  return -4;
    if ((rbpx > xLen) || (rbpx > yLen))  return -5;

    memset((char *)matrix, 0, len * sizeof(double));

    /* 标记左上点所在框, 左上原则 */
    for (index_x = 0; index_x < xGrids; index_x++)
    {
        if (((index_x * xGap) <= ltpx) && (ltpx < ((index_x + 1) * xGap)))
        {
            for (index_y = 0; index_y < yGrids; index_y++)
            {
                if (((index_y * yGap) <= ltpy) && (ltpy < ((index_y + 1) * yGap)))
                {
                    _ltpx = index_x;
                    _ltpy = index_y;

                    dt_x1 = (index_x + 1) * xGap - ltpx;
                    dt_y1 = (index_y + 1) * yGap - ltpy;

                    break;
                }
            }
            break;
        }
    }

    /* 标记右下点所在框 */
    for (index_x = 0; index_x < xGrids; index_x++)
    {
        if (((index_x * xGap) < rbpx) && (rbpx <= ((index_x + 1) * xGap)))
        {
            for (index_y = 0; index_y < yGrids; index_y++)
            {
                if (((index_y * yGap) < rbpy) && (rbpy <= ((index_y + 1) * yGap)))
                {
                    _rbpx = index_x;
                    _rbpy = index_y;

                    dt_x2 = rbpx - (index_x * xGap);
                    dt_y2 = rbpy - (index_y * yGap);

                    break;
                }
            }
            break;
        }
    }

    grid_dt_x = _rbpx - _ltpx;
    grid_dt_y = _rbpy - _ltpy;

    /* 计算左上、右下点所围成的框的面积 */
    if ((box_x > dt_x1) && (box_y > dt_y1))
    {
        for (index_y = 0; index_y < (grid_dt_y + 1); index_y++)
        {
            for (index_x = 0; index_x < (grid_dt_x + 1); index_x++)
            {
                if (index_y == 0)
                {
                    if (index_x == 0)
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = dt_y1 * dt_x1;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = dt_y1 * dt_x1;
#endif
                    }
                    else if (index_x == grid_dt_x)
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = dt_y1 * dt_x2;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = dt_y1 * dt_x2;
#endif
                    }
                    else
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = dt_y1 * xGap;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = dt_y1 * xGap;
#endif
                    }
                }
                else if (index_y == grid_dt_y)
                {
                    if (index_x == 0)
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = dt_y2 * dt_x1;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = dt_y2 * dt_x1;
#endif
                    }
                    else if (index_x == grid_dt_x)
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = dt_y2 * dt_x2;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = dt_y2 * dt_x2;
#endif
                    }
                    else
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = dt_y2 * xGap;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = dt_y2 * xGap;
#endif
                    }
                }
                else
                {
                    if (index_x == 0)
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = yGap * dt_x1;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = yGap * dt_x1;
#endif
                    }
                    else if (index_x == grid_dt_x)
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = yGap * dt_x2;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = yGap * dt_x2;
#endif
                    }
                    else
                    {
#ifndef TWO_DIMENSIONAL_ARRAY
                        matrix[(_ltpy + index_y) * xGrids + _ltpx + index_x] = yGap * xGap;
#else
                        matrix[_ltpy + index_y][_ltpx + index_x] = yGap * xGap;
#endif
                    }
                }
            }
        }
    }
    else if ((box_x <= dt_x1) && (box_y >  dt_y1))
    {
        for (index_y = 0; index_y < (grid_dt_y + 1); index_y++)
        {
            if (index_y == 0)
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[(_ltpy + index_y) * xGrids + _ltpx] = box_x * dt_y1;
#else
                matrix[_ltpy + index_y][_ltpx] = box_x * dt_y1;
#endif
            }
            else if (index_y == grid_dt_y)
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[(_ltpy + index_y) * xGrids + _ltpx] = box_x * dt_y2;
#else
                matrix[_ltpy + index_y][_ltpx] = box_x * dt_y2;
#endif
            }
            else
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[(_ltpy + index_y) * xGrids + _ltpx] = box_x * yGap;
#else
                matrix[_ltpy + index_y][_ltpx] = box_x * yGap;
#endif
            }
        }
    }
    else if ((box_x >  dt_x1) && (box_y <= dt_y1))
    {
        for (index_x = 0; index_x < (grid_dt_x + 1); index_x++)
        {
            if (index_x == 0)
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[_ltpy * xGrids + _ltpx + index_x] = box_y * dt_x1;
#else
                matrix[_ltpy][_ltpx + index_x] = box_y * dt_x1;
#endif
            }
            else if (index_x == grid_dt_x)
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[_ltpy * xGrids + _ltpx + index_x] = box_y * dt_x2;
#else
                matrix[_ltpy][_ltpx + index_x] = box_y * dt_x2;
#endif
            }
            else
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[_ltpy * xGrids + _ltpx + index_x] = box_y * xGap;
#else
                matrix[_ltpy][_ltpx + index_x] = box_y * xGap;
#endif
            }
        }
    }
    else if ((box_x <= dt_x1) && (box_y <= dt_y1))
    {
#ifndef TWO_DIMENSIONAL_ARRAY
        matrix[_ltpy * xGrids + _ltpx] = box_x * box_y;
#else
        matrix[_ltpy][_ltpx] = box_x * box_y;
#endif
    }
    else
    {
        return -6;
    }

    /* 计算左上、右下点所围成的框的面积占比 */
    for (index_x = 0; index_x < xGrids; index_x++)
    {
        for (index_y = 0; index_y < yGrids; index_y++)
        {
            if ((index_x < (xGrids - 1)) && (index_y < (yGrids - 1)))
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[index_y * xGrids + index_x] /= (double)(xGap * yGap);
#else
                matrix[index_y][index_x] /= (double)(xGap * yGap);
#endif
            }
            else if ((index_x == (xGrids - 1)) && (index_y < (yGrids - 1)))
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[index_y * xGrids + index_x] /= (double)(yGap * xRemain);
#else
                matrix[index_y][index_x] /= (double)(yGap * xRemain);
#endif
            }
            else if ((index_x < (xGrids - 1)) && (index_y == (yGrids - 1)))
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[index_y * xGrids + index_x] /= (double)(xGap * yRemain);
#else
                matrix[index_y][index_x] /= (double)(xGap * yRemain);
#endif
            }
            else if ((index_x == (xGrids - 1)) && (index_y == (yGrids - 1)))
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[index_y * xGrids + index_x] /= (double)(xRemain * yRemain);
#else
                matrix[index_y][index_x] /= (double)(xRemain * yRemain);
#endif
            }
            else
            {
                return -7;
            }
        }
    }

    /* 标记左上、右下点所围成的框 */
    for (index_x = 0; index_x < xGrids; index_x++)
    {
        for (index_y = 0; index_y < yGrids; index_y++)
        {
            if ((_ltpx <= index_x) && (index_x <= _rbpx) && (_ltpy <= index_y) && (index_y <= _rbpy))
            {
#ifndef TWO_DIMENSIONAL_ARRAY
                matrix[index_y * xGrids + index_x] += 0;
#else
                matrix[index_y][index_x] += 0;
#endif
            }
        }
    }

    return xGrids * yGrids;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: matrix_test                                                      */
/* 功能描述: 测试用例                                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void matrix_test(void)
{
    double dd[9] = {1.0, 1.0, 2.0, 2.0, 3.0, 3.0};
    double aa[9] = {1.0, 2.0, 3.0, 2.0, 2.0, 1.0, 3.0, 4.0, 3.0};
    double cc[16] = {1.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 3.0, 4.0, 5.0, 5.0, 3.0, 3.0, 3.0, 3.0, 3.0};
    double kk, mm[16];
    char   expr[][128] =
    {
        {"1F + 1G = 1"},
        {"1F - 1G = 5"},
    };

    char *exp1[] = {expr[0], expr[1]};

    kk = matrix_inverse_of_order_n(cc, 4);

    dbug("kk = %f\r\ncc = ", kk);
    for (int i = 0; i < 16; i++)
    {
        dbug("%f ", cc[i]);
    }
    dbug("\r\n");

    kk = matrix_multiply_of_order_n(aa, 3, 3, dd, 3, 2, mm);

    dbug("kk = %f\r\nmm = ", kk);
    for (int i = 0; i < kk; i++)
    {
        dbug("%f ", mm[i]);
    }
    dbug("\r\n");

    matrix_solution_of_order_n(exp1, 2, mm);

    import_matrix_express_from_file();

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: random_point_test                                                */
/* 功能描述: 测试用例                                                         */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-26              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
void random_point_test(void)
{
    box box =
    {
        . ltpx = 5,
        . ltpy = 5,
        . rbpx = 13,
        . rbpy = 26,
        . xLen = 27,
        . yLen = 27,
        . xGap = 5,
        . yGap = 5,
    };

    double *pMatrix = malloc(1024);
    int len, step = (box.xLen % box.xGap) ? (box.xLen / box.xGap + 1) : (box.xLen / box.xGap);

    len = range_and_rate_by_two_points(&box, pMatrix, 1024 / sizeof(double));

    dbug("step = %d, len = %d\r\n", step, len);

    for (int i = 0; i < len; i++)
    {
        if (i % step == 0) dbug("\r\n");
        dbug("%f ", pMatrix[i]);
    }

    free(pMatrix);

    return;
}

