/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: bsp_i2c.c                                                        */
/* 内容摘要: I2C驱动初始化源文件(兼容硬件I2C控制器和软件IO模拟)              */
/* 其它说明: 无                                                               */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/

/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include <string.h>
#include "bsp_i2c.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#ifdef   BSP_USE_SOFT_I2C       /* (SDA与SCL须与msp.c中保持一致) */
#define  I2C1_PORT              GPIOB
#define  I2C1_SCL               GPIO_PIN_8
#define  I2C1_SDA               GPIO_PIN_9

#define  I2C2_PORT              GPIOB
#define  I2C2_SCL               GPIO_PIN_10
#define  I2C2_SDA               GPIO_PIN_11

#define  I2C3_SCL_PORT          GPIOA
#define  I2C3_SCL               GPIO_PIN_8
#define  I2C3_SDA_PORT          GPIOC
#define  I2C3_SDA               GPIO_PIN_9

#define  I2C_SDA_H(X)           HAL_GPIO_WritePin(X->i2c_cfg->sda_port, \
                                                  X->i2c_cfg->sda_pin,  \
                                                  GPIO_PIN_SET)
#define  I2C_SDA_L(X)           HAL_GPIO_WritePin(X->i2c_cfg->sda_port, \
                                                  X->i2c_cfg->sda_pin,  \
                                                  GPIO_PIN_RESET)

#define  I2C_SCL_H(X)           HAL_GPIO_WritePin(X->i2c_cfg->scl_port, \
                                                  X->i2c_cfg->scl_pin,  \
                                                  GPIO_PIN_SET)
#define  I2C_SCL_L(X)           HAL_GPIO_WritePin(X->i2c_cfg->scl_port, \
                                                  X->i2c_cfg->scl_pin,  \
                                                  GPIO_PIN_RESET)

#define  I2C_SDA_VAL(X)         HAL_GPIO_ReadPin(X->i2c_cfg->sda_port,  \
                                                 X->i2c_cfg->sda_pin)
#define  I2C_SCL_VAL(X)         HAL_GPIO_ReadPin(X->i2c_cfg->scl_port,  \
                                                 X->i2c_cfg->scl_pin)

#define  I2C_HOLD_DELAY(X)      Fn_hw_us_delay(X->i2c_cfg->holdtime)        /* 保持延时 */
#endif

#ifdef  BSP_USE_RT_THREAD
#define  I2C_WRCY_DELAY(X)      rt_thread_mdelay(X->i2c_cfg->wrcycle)       /* 写周期时间 */
#else
#define  I2C_WRCY_DELAY(X)      HAL_Delay(X->i2c_cfg->wrcycle)
#endif

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/
/*<ENUM+>**********************************************************************/
/* 枚举: _I2C_BUS_DEV_INDEX                                                   */
/* 注释: I2C总线设备枚举                                                      */
/*<ENUM->**********************************************************************/
enum _i2c_bus_dev_index
{
#ifdef BSP_USE_I2C1
    I2C1_INDEX,
#endif

#ifdef BSP_USE_I2C2
    I2C2_INDEX,
#endif

#ifdef BSP_USE_I2C3
    I2C3_INDEX,
#endif
};

/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static void bsp_i2c_ev_isr(enum _i2c_bus_dev_index index);
static void bsp_i2c_er_isr(enum _i2c_bus_dev_index index);

#ifdef BSP_USE_SOFT_I2C
static int  i2c_reset(struct _i2c_bus_dev *i2c_bus);
static int  i2c_start(struct _i2c_bus_dev *i2c_bus);
static int  i2c_restart(struct _i2c_bus_dev *i2c_bus);
static int  i2c_stop(struct _i2c_bus_dev *i2c_bus);
static void i2c_ack(struct _i2c_bus_dev *i2c_bus);
static void i2c_nack(struct _i2c_bus_dev *i2c_bus);
static int  i2c_waitack(struct _i2c_bus_dev *i2c_bus);
static int  i2c_writebyte(struct _i2c_bus_dev *i2c_bus, unsigned char byte);
static unsigned char i2c_readbyte(struct _i2c_bus_dev *i2c_bus);
static int  i2c_sendaddr(struct _i2c_bus_dev *i2c_bus, struct _i2c_bus_msg *msg);
static int  i2c_recv_bytes(struct _i2c_bus_dev *i2c_bus, struct _i2c_bus_msg *msg);
static int  i2c_send_bytes(struct _i2c_bus_dev *i2c_bus, struct _i2c_bus_msg *msg);
#endif

static int _i2c_bus_init(struct device *dev);
static int _i2c_bus_open(struct device *dev, int flag);
static int _i2c_bus_close(struct device *dev);
static int _i2c_bus_xfer(struct device *dev, void *msgs, int num);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
#ifdef BSP_USE_I2C1
static I2C_HandleTypeDef hi2c1;                                /* 设备实体 */
struct _i2c_bus_hal i2c_bus_dev1_hal = 
{
    .i2cx  = I2C1,
    .irqx1 = I2C1_EV_IRQn,
    .irqx2 = I2C1_ER_IRQn,
    .hi2cx = &hi2c1,
};

struct _i2c_bus_cfg i2c_bus_dev1_cfg =                  
{
#ifdef BSP_USE_HARD_I2C
    .baud      = 400 * 1000,
    .duty      = I2C_DUTYCYCLE_16_9,
    .addrwidth = I2C_ADDRESSINGMODE_7BIT,
#endif

#ifdef BSP_USE_SOFT_I2C
    .sda_port = I2C1_PORT,
    .sda_pin  = I2C1_SDA,
    .scl_port = I2C1_PORT,
    .scl_pin  = I2C1_SCL,
#endif
};

static struct _i2c_bus_dev i2c_bus_dev1 =
{
    .i2c_name = "i2c1",
    .i2c_hal  = &i2c_bus_dev1_hal,
    .i2c_cfg  = &i2c_bus_dev1_cfg,
};
#endif

#ifdef BSP_USE_I2C2
static I2C_HandleTypeDef hi2c2;
struct _i2c_bus_hal i2c_bus_dev2_hal = 
{
    .i2cx  = I2C2,
    .irqx1 = I2C2_EV_IRQn,
    .irqx2 = I2C2_ER_IRQn,
    .hi2cx = &hi2c2,
};

struct _i2c_bus_cfg i2c_bus_dev2_cfg =                  
{
#ifdef BSP_USE_HARD_I2C
    .baud      = 300 * 1000,
    .duty      = I2C_DUTYCYCLE_2,
    .addrwidth = I2C_ADDRESSINGMODE_7BIT,
#endif

#ifdef BSP_USE_SOFT_I2C
    .sda_port = I2C2_PORT,
    .sda_pin  = I2C2_SDA,
    .scl_port = I2C2_PORT,
    .scl_pin  = I2C2_SCL,
#endif
};

static struct _i2c_bus_dev i2c_bus_dev2 =
{
    .i2c_name = "i2c2",
    .i2c_hal  = &i2c_bus_dev2_hal,
    .i2c_cfg  = &i2c_bus_dev2_cfg,
};
#endif

#ifdef BSP_USE_I2C3
static I2C_HandleTypeDef hi2c3;
struct _i2c_bus_hal i2c_bus_dev3_hal = 
{
    .i2cx  = I2C3,
    .irqx1 = I2C3_EV_IRQn,
    .irqx2 = I2C3_ER_IRQn,
    .hi2cx = &hi2c3,
};

struct _i2c_bus_cfg i2c_bus_dev3_cfg =                  
{
#ifdef BSP_USE_HARD_I2C
    .baud      = 200 * 1000,
    .duty      = I2C_DUTYCYCLE_16_9,
    .addrwidth = I2C_ADDRESSINGMODE_10BIT,
#endif

#ifdef BSP_USE_SOFT_I2C
    .sda_port = I2C3_SDA_PORT,
    .sda_pin  = I2C3_SDA,
    .scl_port = I2C3_SCL_PORT,
    .scl_pin  = I2C3_SCL,
#endif
};

static struct _i2c_bus_dev i2c_bus_dev3 =
{
    .i2c_name = "i2c3",
    .i2c_hal  = &i2c_bus_dev3_hal,
    .i2c_cfg  = &i2c_bus_dev3_cfg,
};
#endif

static struct _i2c_bus_dev *i2c_bus_devs[] =
{
#ifdef BSP_USE_I2C1
    &i2c_bus_dev1,
#endif

#ifdef BSP_USE_I2C2
    &i2c_bus_dev2,
#endif

#ifdef BSP_USE_I2C3
    &i2c_bus_dev3,
#endif
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
#ifdef BSP_USE_SOFT_I2C
/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_reset                                                        */
/* 功能描述: I2C总线复位                                                      */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: AT24Cxx需要9个周期复位总线                                      */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_reset(struct _i2c_bus_dev *i2c_bus)
{
    int i;
  
    if (0 == I2C_SDA_VAL(i2c_bus))
    {
        for (i = 0; i < 9; i++)
        {
            I2C_SCL_L(i2c_bus);
            Fn_hw_us_delay(100);
            I2C_SCL_H(i2c_bus);
            Fn_hw_us_delay(100);
        }
    }

    if (0 == I2C_SDA_VAL(i2c_bus))
    {
        return -1;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_start                                                        */
/* 功能描述: I2C总线起始信号                                                  */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 起始前保证SDA,SCL均为高电平,起始后保证SDA为低,SCL为高           */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_start(struct _i2c_bus_dev *i2c_bus)
{
    I2C_HOLD_DELAY(i2c_bus);        /* SDA,SCL保持5us高电平,tSU.STA */

    if ((!I2C_SCL_VAL(i2c_bus)) || (!I2C_SDA_VAL(i2c_bus)))
    {
        if (i2c_reset(i2c_bus))
        {
            return -1;
        }
    }

    I2C_SDA_L(i2c_bus);             /* 拉低SDA */
    I2C_HOLD_DELAY(i2c_bus);        /* SDA保持5us低电平,tHD.STA */

    I2C_SCL_L(i2c_bus);             /* 拉低SCL,为后续SDA变化做准备 */
    __NOP(); __NOP(); __NOP();      /* t.AA = 100ns */
    __NOP(); __NOP(); __NOP();

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_restart                                                      */
/* 功能描述: I2C总线重新起始信号                                              */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 各操作均以SCL为高电平结尾                                       */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_restart(struct _i2c_bus_dev *i2c_bus)
{
    I2C_SCL_L(i2c_bus);             /* 各操作均以SCL为高结尾 */
    I2C_SDA_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SCL_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SDA_L(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SCL_L(i2c_bus);
    __NOP(); __NOP(); __NOP();      /* t.AA = 100ns */
    __NOP(); __NOP(); __NOP();

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_writebyte                                                    */
/* 功能描述: I2C总线写字节                                                    */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/*           byte --- 写入字节                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程: SCL周期:以低电平持续5us开始,以高电平持续5us结束                */
/* 其它说明: 完整重复的SCL周期为:拉低-延时-拉高-延时,写之前SCL为高电平      */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_writebyte(struct _i2c_bus_dev *i2c_bus, unsigned char byte)
{
    unsigned int  i;
    unsigned char ch = byte;

    for (i = 0; i < 8; i++, ch <<= 1)
    {
        I2C_SCL_L(i2c_bus);

        __NOP(); __NOP(); __NOP();                                /* tHD.DAT */

        (ch & 0x80) ? I2C_SDA_H(i2c_bus) : I2C_SDA_L(i2c_bus);    /* 调整数据 */

        I2C_HOLD_DELAY(i2c_bus);                                  /* tLOW + tSU.DAT */

        I2C_SCL_H(i2c_bus);

        I2C_HOLD_DELAY(i2c_bus);                                  /* tHIGH */
    }

    return i2c_waitack(i2c_bus);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_waitack                                                      */
/* 功能描述: I2C总线等待ACK                                                   */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 0: 等到从机ACK; -1: 超时未等到从机ACK,此时SDA仍为高电平         */
/* 返 回 值: static int                                                       */
/* 操作流程: 第9个SCL周期内主机无法控制SDA,完全由从机控制,主机只负责产生    */
/* 其它说明: 一个SCL即可,因此在此周期内操作SDA毫无意义,结束后SCL为高        */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_waitack(struct _i2c_bus_dev *i2c_bus)
{
    I2C_SCL_L(i2c_bus);
    I2C_SDA_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SCL_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    return I2C_SDA_VAL(i2c_bus);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_readbyte                                                     */
/* 功能描述: I2C总线读字节                                                    */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 返回读到的字节数据                                              */ 
/* 返 回 值: static unsigned char                                             */
/* 操作流程:                                                                  */
/* 其它说明: 注意总线速率与等待时间的匹配                                    */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static unsigned char i2c_readbyte(struct _i2c_bus_dev *i2c_bus)
{
    unsigned int i;
    unsigned char byte = 0;

    for (i = 0; i < 8; i++)
    {
        I2C_SCL_L(i2c_bus);             /* 为之后从机操作SDA,先拉低SCL */
        I2C_SDA_H(i2c_bus);             /* 拉高SDA,等待从机操作SDA */
        I2C_HOLD_DELAY(i2c_bus);

        I2C_SCL_H(i2c_bus);
        I2C_HOLD_DELAY(i2c_bus);

        byte <<= 1;
        byte |= I2C_SDA_VAL(i2c_bus) ? 1 : 0;   /* 等待10us后采样, MAX = 100KHz */
    }

    return byte;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_ack                                                          */
/* 功能描述: I2C总线发送ACK                                                   */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void i2c_ack(struct _i2c_bus_dev *i2c_bus)
{
    I2C_SCL_L(i2c_bus);
    I2C_SDA_L(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SCL_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_nack                                                         */
/* 功能描述: I2C总线发送NACK                                                  */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void i2c_nack(struct _i2c_bus_dev *i2c_bus)
{
    I2C_SCL_L(i2c_bus);
    I2C_SDA_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SCL_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_stop                                                         */
/* 功能描述: I2C总线终止信号                                                  */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: stop衔接下次的start,因此总线为高电平空闲状态                    */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_stop(struct _i2c_bus_dev *i2c_bus)
{
    I2C_SCL_L(i2c_bus);
    I2C_SDA_L(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);

    I2C_SCL_H(i2c_bus);
    I2C_HOLD_DELAY(i2c_bus);        /* tSU.STO */

    I2C_SDA_H(i2c_bus);
    I2C_WRCY_DELAY(i2c_bus);        /* tBUF + 重新启动前的写操作时间twr */

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_sendaddr                                                     */
/* 功能描述: I2C总线发送设备地址数据                                          */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/*           msg --- I2C数据结构体                                            */
/* 输出参数: 已发送的长度                                                     */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_sendaddr(struct _i2c_bus_dev *i2c_bus, struct _i2c_bus_msg *msg)
{
    int ret;

    if (msg->flag & BSP_I2C_ADDR_10BIT)
    {
        unsigned char addr1, addr2;

        addr1 = 0xF0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xFF;

        ret = i2c_writebyte(i2c_bus, addr1);
        if (ret != 0)
        {
            return -1;
        }

        ret = i2c_writebyte(i2c_bus, addr2);
        if (ret != 0)
        {
            return -1;
        }

        if (msg->flag & BSP_I2C_RD)
        {
            i2c_restart(i2c_bus);

            addr1 |= 0x01;

            ret = i2c_writebyte(i2c_bus, addr1);
            if (ret != 0)
            {
                return -1;
            }
        }
    }
    else  /* 7-bit addr */
    {
        unsigned char addr;

        addr = msg->addr << 1 | ((msg->flag & BSP_I2C_RD) ? 1 : 0);  /* 0x50 -> 0xA0 */

        ret = i2c_writebyte(i2c_bus, addr);
        if (ret != 0)
        {
            return -1;
        }
    }

    return ret;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_recv_bytes                                                   */
/* 功能描述: I2C总线接收数据                                                  */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/*           msg --- I2C数据结构体                                            */
/* 输出参数: 已接收的长度                                                     */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_recv_bytes(struct _i2c_bus_dev *i2c_bus, struct _i2c_bus_msg *msg)
{
    int len = (int)msg->len;
    unsigned char *ptr = msg->buf;

    while (len-- > 0)
    {
        *ptr++ = i2c_readbyte(i2c_bus);

        (len > 0) ? i2c_ack(i2c_bus) : i2c_nack(i2c_bus);
    }

    return (int)msg->len;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: i2c_send_bytes                                                   */
/* 功能描述: I2C总线发送数据                                                  */
/* 输入参数: i2c_bus --- I2C设备句柄                                          */
/*           msg --- I2C数据结构体                                            */
/* 输出参数: 已发送的长度                                                     */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int i2c_send_bytes(struct _i2c_bus_dev *i2c_bus, struct _i2c_bus_msg *msg)
{
    int ret;
    int len = (int)msg->len;
    unsigned char *ptr = msg->buf;

    while (len-- > 0)
    {
        ret = i2c_writebyte(i2c_bus, *ptr++);

        if (ret != 0)
        {
            return -1;
        }
    }

    return (int)msg->len;
}
#endif

/*<FUNC+>**********************************************************************/
/* 函数名称: _i2c_bus_init                                                    */
/* 功能描述: I2C总线操作之初始化                                              */
/* 输入参数: dev --- I2C总线设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _i2c_bus_init(struct device *dev)
{
    struct _i2c_bus_dev *i2c_bus = (struct _i2c_bus_dev *)dev;
    assert_param(i2c_bus);
    
#ifdef BSP_USE_HARD_I2C
    i2c_bus->i2c_hal->hi2cx->Instance = i2c_bus->i2c_hal->i2cx;
    i2c_bus->i2c_hal->hi2cx->Init.ClockSpeed = i2c_bus->i2c_cfg->baud;
    i2c_bus->i2c_hal->hi2cx->Init.DutyCycle = i2c_bus->i2c_cfg->duty;
    i2c_bus->i2c_hal->hi2cx->Init.AddressingMode = i2c_bus->i2c_cfg->addrwidth;
    i2c_bus->i2c_hal->hi2cx->Init.OwnAddress1 = 0;
    i2c_bus->i2c_hal->hi2cx->Init.OwnAddress2 = 0;
    i2c_bus->i2c_hal->hi2cx->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c_bus->i2c_hal->hi2cx->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2c_bus->i2c_hal->hi2cx->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(i2c_bus->i2c_hal->hi2cx) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    if (HAL_I2CEx_ConfigAnalogFilter(i2c_bus->i2c_hal->hi2cx, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    if (HAL_I2CEx_ConfigDigitalFilter(i2c_bus->i2c_hal->hi2cx, 0) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
#endif
    
#ifdef BSP_USE_SOFT_I2C
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;    /* must this */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;            /* must this */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin = i2c_bus->i2c_cfg->scl_pin;
    HAL_GPIO_Init(i2c_bus->i2c_cfg->scl_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = i2c_bus->i2c_cfg->sda_pin;
    HAL_GPIO_Init(i2c_bus->i2c_cfg->sda_port, &GPIO_InitStruct);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _i2c_bus_open                                                    */
/* 功能描述: I2C总线操作之开启                                                */
/* 输入参数: dev --- I2C总线设备句柄                                          */
/*           flag --- 打开参数                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _i2c_bus_open(struct device *dev, int flag)
{
    struct _i2c_bus_dev *i2c_bus = (struct _i2c_bus_dev *)dev;
    assert_param(i2c_bus);

#ifdef  BSP_USE_HARD_I2C
    HAL_NVIC_EnableIRQ(i2c_bus->i2c_hal->irqx1);
    HAL_NVIC_EnableIRQ(i2c_bus->i2c_hal->irqx2);
    __HAL_I2C_ENABLE(i2c_bus->i2c_hal->hi2cx);
#endif
    
#ifdef  BSP_USE_SOFT_I2C    /* 总线空闲态,SDA与SCL均为高 */
    HAL_GPIO_WritePin(i2c_bus->i2c_cfg->sda_port, \
                      i2c_bus->i2c_cfg->sda_pin,  \
                      GPIO_PIN_SET);
    HAL_GPIO_WritePin(i2c_bus->i2c_cfg->scl_port, \
                      i2c_bus->i2c_cfg->scl_pin,  \
                      GPIO_PIN_SET);
#endif
    
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _i2c_bus_close                                                   */
/* 功能描述: I2C总线操作之关闭                                                */
/* 输入参数: dev --- I2C总线设备句柄                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _i2c_bus_close(struct device *dev)
{
    struct _i2c_bus_dev *i2c_bus = (struct _i2c_bus_dev *)dev;
    assert_param(i2c_bus);

#ifdef  BSP_USE_HARD_I2C
    HAL_NVIC_DisableIRQ(i2c_bus->i2c_hal->irqx1);
    HAL_NVIC_DisableIRQ(i2c_bus->i2c_hal->irqx2);
    __HAL_I2C_DISABLE(i2c_bus->i2c_hal->hi2cx);
#endif
    
#ifdef  BSP_USE_SOFT_I2C
    HAL_GPIO_DeInit(i2c_bus->i2c_cfg->sda_port, i2c_bus->i2c_cfg->sda_pin);
    HAL_GPIO_DeInit(i2c_bus->i2c_cfg->scl_port, i2c_bus->i2c_cfg->scl_pin);
#endif

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: _i2c_bus_xfer                                                    */
/* 功能描述: I2C总线读写统一操作函数                                          */
/* 输入参数: dev --- I2C总线设备句柄                                          */
/*           msgs --- I2C数据结构体指针                                       */
/*           num --- I2C数据结构体个数                                        */
/* 输出参数: 读入/写出的数据长度                                             */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-29              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int _i2c_bus_xfer(struct device *dev, void *msgs, int num)
{
    int i, ret = 0;
    struct _i2c_bus_msg *msg = (struct _i2c_bus_msg *)msgs;
    struct _i2c_bus_dev *i2c_bus = (struct _i2c_bus_dev *)dev;
    
    assert_param(msg);
    assert_param(i2c_bus);
    assert_param(2 == num);

#ifdef  BSP_USE_SOFT_I2C
    i2c_start(i2c_bus);

    for (i = 0; i < num; i++)
    {
        if (!(msg[i].flag & BSP_I2C_NOSTART))
        {
            if (i)
            {
                i2c_restart(i2c_bus);
            }

            ret = i2c_sendaddr(i2c_bus, &msg[i]);

            if (ret != 0)
            {
                goto EXIT_LABEL;
            }
        }

        if (msg[i].flag & BSP_I2C_RD)
        {
            ret = i2c_recv_bytes(i2c_bus, &msg[i]);

            if (ret < 1)
            {
                goto EXIT_LABEL;
            }
        }
        else if (msg[i].flag & BSP_I2C_WR)
        {
            ret = i2c_send_bytes(i2c_bus, &msg[i]);

            if (ret < 1)
            {
                goto EXIT_LABEL;
            }
        }
        else
        {
            Error_Handler(__FILE__, __LINE__);
        }
    }

    ret = i;

EXIT_LABEL:
    i2c_stop(i2c_bus);
#endif

#ifdef  BSP_USE_HARD_I2C
    int MemAddress = 0;

    for (i = 0; i < num; i++)
    {
        if ((NULL == msg[i].buf) || (msg[i].len <= 0))
        {
             return -1;
        }
    }

    for (i = 0; i < msg[0].len; i++)
    {
        MemAddress <<= 8;
        MemAddress |= msg[0].buf[i];
    }

    if (msg[1].flag & BSP_I2C_RD)
    {                                                   /* 0x50 -> 0xA0 */
        ret = HAL_I2C_Mem_Read(i2c_bus->i2c_hal->hi2cx, msg[0].addr << 1, \
                               MemAddress, i2c_bus->i2c_cfg->datawidth,   \
                               msg[1].buf, msg[1].len, 10);

        I2C_WRCY_DELAY(i2c_bus);                        /* tWR必须存在 */  
    }
    else if (msg[1].flag & BSP_I2C_WR)
    {
        ret = HAL_I2C_Mem_Write(i2c_bus->i2c_hal->hi2cx, msg[0].addr << 1, \
                                MemAddress, i2c_bus->i2c_cfg->datawidth,   \
                                msg[1].buf, msg[1].len, 10);

        I2C_WRCY_DELAY(i2c_bus);
    }
    else
    {
        return -2;
    }
    
    ret = (HAL_OK == ret) ? msg[1].len : -3;
#endif

    return ret;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_i2c_init                                                     */
/* 功能描述: I2C总线初始化和注册函数                                          */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int bsp_i2c_init(void)
{   
    int   i;
    const char   *name = NULL;
    struct device *dev = NULL;

    for (i = 0; i < sizeof(i2c_bus_devs) / sizeof(struct _i2c_bus_dev *); i++)
    {
        name = i2c_bus_devs[i]->i2c_name;
        dev = &i2c_bus_devs[i]->i2c_dev;
        
        assert_param(sizeof(name) <= DEVICE_NAME_MAXLEN);
        strcpy(dev->name, name);
        
        dev->init  = _i2c_bus_init;
        dev->open  = _i2c_bus_open;
        dev->close = _i2c_bus_close;
        dev->xfer  = _i2c_bus_xfer;  

        if(0 != Fn_device_register(dev, name))
        {
            Error_Handler(__FILE__, __LINE__);
        }  

        if(0 != Fn_device_open(dev, 0))
        {
            Error_Handler(__FILE__, __LINE__);
        }
    }

    return 0;
}

int _bsp_i2c_init(void)
{
    return bsp_i2c_init();
}
INIT_BOARD_EXPORT(_bsp_i2c_init);

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_i2c_ev_isr                                                   */
/* 功能描述: I2C_EV中断统一处理函数                                           */
/* 输入参数: index --- I2C索引                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void bsp_i2c_ev_isr(enum _i2c_bus_dev_index index)
{
#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_enter(); 
#else
    __disable_irq();
#endif

    if (__HAL_I2C_GET_FLAG(i2c_bus_devs[index]->i2c_hal->hi2cx, I2C_FLAG_RXNE))
    {

    }
    else if (__HAL_I2C_GET_FLAG(i2c_bus_devs[index]->i2c_hal->hi2cx, I2C_FLAG_TXE))
    {

    }

#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_leave(); 
#else
    __enable_irq();
#endif

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: bsp_i2c_er_isr                                                   */
/* 功能描述: I2C_ER中断统一处理函数                                           */
/* 输入参数: index --- I2C索引                                                */
/* 输出参数: 无                                                               */
/* 返 回 值: static void                                                      */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-12-10              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static void bsp_i2c_er_isr(enum _i2c_bus_dev_index index)
{
#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_enter(); 
#else
    __disable_irq();
#endif

    if (__HAL_I2C_GET_FLAG(i2c_bus_devs[index]->i2c_hal->hi2cx, I2C_FLAG_RXNE))
    {

    }
    else if (__HAL_I2C_GET_FLAG(i2c_bus_devs[index]->i2c_hal->hi2cx, I2C_FLAG_TXE))
    {

    }

#ifdef  BSP_USE_RT_THREAD
    rt_interrupt_leave(); 
#else
    __enable_irq();
#endif

    return;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: I2Cx_IRQHandler                                                  */
/* 功能描述: I2C中断入口函数                                                  */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: void                                                             */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
#ifdef  BSP_USE_I2C1
void I2C1_EV_IRQHandler(void)
{
    bsp_i2c_ev_isr(I2C1_INDEX);
}
void I2C1_ER_IRQHandler(void)
{
    bsp_i2c_er_isr(I2C1_INDEX);
}
#endif

#ifdef  BSP_USE_I2C2
void I2C2_EV_IRQHandler(void)
{
    bsp_i2c_ev_isr(I2C2_INDEX);
}
void I2C2_ER_IRQHandler(void)
{
    bsp_i2c_er_isr(I2C2_INDEX);
}
#endif

#ifdef  BSP_USE_I2C3
void I2C3_EV_IRQHandler(void)
{
    bsp_i2c_ev_isr(I2C3_INDEX);
}
void I2C3_ER_IRQHandler(void)
{
    bsp_i2c_er_isr(I2C3_INDEX);
}
#endif
