/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) Leonard.  2020. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_common.c                                                      */
/* 内容摘要: 系统通用函数源文件(兼容OTA+IAP实现)                             */
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
#include <stdlib.h>
#include "bsp_flash.h"
#include "Fn_format.h"
#include "Fn_common.h"
#include "Fn_command.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
#define TIMER_LED                   (0x01)              /* LED定时器索引ID   */
#define TIMER_WAIT                  (0x02)              /* 等待定时器索引ID  */
#define TIMER_LOAD                  (0x03)              /* 加载定时器索引ID  */

#define OUTSIDE_FLASH               "w25q64"            /* 外部FLASH设备名称 */
#define INSIDE_FLASH                "flash"             /* 内部FLASH设备名称 */

/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/
static int led_blink(void *para);
static int load_callback(void *para);
static int wait_callback(void *para);

static int IAP_JumpToApp(unsigned int addr);
static int IAP_OperUpdateInfo(unsigned char *idata, int ilen, OPER_DIR rw);
static int IAP_OperBootInfo(unsigned char *idata, int ilen, OPER_DIR rw);
static int IAP_CodeCarry(unsigned int src_addr, unsigned int obj_addr, int size, CARRY_DIR dir);
static int IAP_CodeCheck(const char *name, unsigned int area_addr, unsigned int size);

/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static struct _sys_param Sys_Param;                        /* 全局参数结构体 */
static char   _wait_flag[4] = {'\\', '|', '/', '-'};       /* 等待打印提示   */
static char  *_boot_cmd = "#####";                         /* 进入CMD模式    */                
static struct _timer_dev TimerTable[] =                    /* 软件定时器列表 */
{
    { TIMER_LED,     TIMER_ENABLE,   TIMER_CIRCLE,  200,   led_blink          },
    { TIMER_LOAD,    TIMER_DISABLE,  TIMER_CIRCLE,  150,   load_callback      },
    { TIMER_WAIT,    TIMER_DISABLE,  TIMER_SINGLE,  5000,  wait_callback      },
}; 

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
static int led_blink(void *para)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    return 0;
}

static int load_callback(void *para)
{
    static unsigned int cunt = 0;

    return dbug("\b%c", _wait_flag[cunt++ % 4]);
}

static int wait_callback(void *para)
{
    return Fn_timer_control(TIMER_LOAD, TIMER_NEW_STATE, (int *)TIMER_DISABLE);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: IAP_JumpToApp                                                    */
/* 功能描述: 从boot跳转到主程序                                               */
/* 输入参数: addr --- 将要跳转的地址                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: >0:成功                                                          */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int IAP_JumpToApp(unsigned int addr)
{
    unsigned int new_msp;
    unsigned int JumpAPP;

    if (addr != APP_FLASH_START)
    {
        return -1;
    }

    __disable_irq();   /* 关闭总中断 */
    
    /**************************************************************************/
    /* 检查首地址上的指令是否合法                                            */
    /**************************************************************************/
    if (((*(volatile unsigned int *)addr) & (~(APP_SRAM_SIZE - 1))) == APP_SRAM_START)
    {
        /**********************************************************************/
        /* 获取新的主栈指针MSP地址                                            */
        /**********************************************************************/
        new_msp = *(unsigned int *)addr;

        /**********************************************************************/
        /* 数据存储器屏障，确保到存储器的写操作结束                          */
        /**********************************************************************/
        __DMB();

        /**********************************************************************/
        /* 设置新的向量表                                                     */
        /**********************************************************************/
        SCB->VTOR = addr;
        
        /**********************************************************************/
        /* 数据同步屏障，确保接下来的所有指令都使用新配置 */
        /**********************************************************************/
        __DSB();

        /**********************************************************************/
        /* 获取复位中断处理函数的位置Reset_Handle                            */
        /**********************************************************************/
        JumpAPP = *(unsigned int *)(addr + 4);
        
        /**********************************************************************/
        /* 设置主栈指针MSP                                                    */
        /**********************************************************************/
        __set_MSP(new_msp);

        /**********************************************************************/
        /* 执行Reset_Handle函数,将整型数转为void (*f)(void)型的函数指针      */
        /**********************************************************************/
        (*(void (*)(void))JumpAPP)();
    }
    
    __enable_irq();    /* 开启总中断 */

    return -2;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: IAP_OperUpdateInfo                                               */
/* 功能描述: 读取/写入IAP更新标志                                             */
/* 输入参数: idata --- 数据区                                                 */
/*           ilen --- 数据长                                                  */
/*           rw --- 读取/写入,  0 : 读取;  !0 : 写入                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 操作成功返回 0                                                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-27              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int IAP_OperUpdateInfo(unsigned char *idata, int ilen, OPER_DIR rw)
{
    struct device *dev = NULL;

    dev = Fn_device_find(OUTSIDE_FLASH);

    if (dev != NULL)
    {
        if (rw == READ)
        {
            return Fn_device_read(dev, EFLASH_IAP_INFO_ADDR, idata, ilen);
        }
        else
        {
            return Fn_device_write(dev, EFLASH_IAP_INFO_ADDR, idata, ilen);
        }
    }
    else
    {
        return -1;
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: IAP_OperBootInfo                                                 */
/* 功能描述: 设置/读取BOOT标识信息                                            */
/* 输入参数: idata --- 数据区                                                 */
/*           ilen --- 数据长                                                  */
/*           rw --- 读取/写入,  0 : 读取;  !0 : 写入                          */
/* 输出参数: 无                                                               */
/* 返 回 值: static int                                                       */
/* 操作流程:                                                                  */
/* 其它说明: 操作成功返回 0                                                  */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-27              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int IAP_OperBootInfo(unsigned char *idata, int ilen, OPER_DIR rw)
{
    struct device *dev = NULL;

    dev = Fn_device_find(OUTSIDE_FLASH);

    if (dev != NULL)
    {
        if (rw == READ)
        {
            return Fn_device_read(dev, EFLASH_BOOT_INFO_ADDR, idata, ilen);
        }
        else
        {
            return Fn_device_write(dev, EFLASH_BOOT_INFO_ADDR, idata, ilen);
        }
    }
    else
    {
        return -1;
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: IAP_CodeCarry                                                    */
/* 功能描述: 程序搬运、刷写                                                   */
/* 输入参数: src_addr --- 源区域起始地址                                      */
/*           obj_addr --- 目标区域起始地址                                    */
/*           size --- 需要拷贝源区域数据大小                                  */
/*           dir --- 数据搬运方向                                             */
/* 输出参数: 无                                                               */
/* 返 回 值: 0 = 成功                                                         */
/* 操作流程:                                                                  */
/* 其它说明: 支持stm32->stm32, stm32->w25q64, w25q64->stm32, w25q64->w25q64   */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int IAP_CodeCarry(unsigned int src_addr, unsigned int obj_addr, int size, CARRY_DIR dir)
{   
      signed int res = 0;
    unsigned int pack = 4096;
    unsigned int packs = 0;
    unsigned int remain = 0; 
    unsigned int already = 0;
    unsigned int src_start, src_end;
    unsigned int obj_start, obj_end;
    unsigned char *pt = NULL;
    struct device *dev1 = NULL;
    struct device *dev2 = NULL;
    char src_name[32] = {0};
    char obj_name[32] = {0};

    switch (dir)
    {
    case STM_TO_STM:
        dev1 = Fn_device_find(INSIDE_FLASH);
        src_start = APP_FLASH_START;
        src_end   = APP_FLASH_END;
        strcpy(src_name, "stm-falsh");
        
        dev2 = Fn_device_find(INSIDE_FLASH);
        obj_start = APP_FLASH_START;
        obj_end   = APP_FLASH_END;
        strcpy(obj_name, "stm-flash");    
        break;

    case STM_TO_W25Q:
        dev1 = Fn_device_find(INSIDE_FLASH);
        src_start = APP_FLASH_START;
        src_end   = APP_FLASH_END;
        strcpy(src_name, "stm-falsh");
        
        dev2 = Fn_device_find(OUTSIDE_FLASH);
        obj_start = EFLASH_IAP_START;
        obj_end   = EFLASH_IAP_END;
        strcpy(obj_name, "w25q-flash");    
        break;

    case W25Q_TO_STM:
        dev1 = Fn_device_find(OUTSIDE_FLASH);
        src_start = EFLASH_IAP_START;
        src_end   = EFLASH_IAP_END;
        strcpy(src_name, "w25q-flash");    

        dev2 = Fn_device_find(INSIDE_FLASH);
        obj_start = APP_FLASH_START;
        obj_end   = APP_FLASH_END;
        strcpy(obj_name, "stm-flash");    
        break;

    case W25Q_TO_W25Q:
        dev1 = Fn_device_find(OUTSIDE_FLASH);
        src_start = EFLASH_IAP_START;
        src_end   = EFLASH_IAP_END;
        strcpy(src_name, "w25q-flash");    

        dev2 = Fn_device_find(OUTSIDE_FLASH);
        obj_start = EFLASH_IAP_START;
        obj_end   = EFLASH_IAP_END;
        strcpy(obj_name, "w25q-flash");    
        break;

    default:
        break;
    }
    
    if ((src_addr != src_start) || ((src_addr + size) > src_end))
    {
        return -1;
    }

    if ((obj_addr != obj_start) || ((obj_addr + size) > obj_end)) 
    {
        return -2;
    }

    if ((size <= 0) || (NULL == dev1) || (NULL == dev2))
    {
        return -3;
    }

    dbug("IAP carry %d bytes code from %s addr 0x%08x to %s addr 0x%08x\r\n", size, src_name, src_start, obj_name, obj_start);

    while(pack)
    {
#ifdef  BSP_USE_RT_THREAD
        pt = (unsigned char *)rt_malloc(pack);
#else
        pt = (unsigned char *)malloc(pack);
#endif
        if(pt == NULL)
        {
            pack >>= 1;
        }
        else
        {
            break;
        }
    }

    if(pack == 0) 
    {
        return -4;
    }

    if(pack >= size)
    {
        pack = size;
        packs = 1;
    }
    else
    {
        remain = size % pack;
        packs  = size / pack + ((remain == 0) ? 0 : 1);
    }

    do
    {
        if (0 == Fn_device_read(dev1, src_addr, pt, pack))
        {
            if (0 == (Fn_device_write(dev2, obj_addr, pt, pack)))
            {
                src_addr += pack;
                obj_addr += pack;
                already  += pack;

                if((remain != 0) && (packs == 2))
                {
                    pack = remain;
                }

                Fn_print_Progressbar("Code-Carry", already, size);
            }
            else
            {
                res = -5;
                goto EXIT_LABEL;
            }
        }
        else
        {
            res = -6;
            goto EXIT_LABEL;
        }
    }while(--packs > 0);

EXIT_LABEL:
#ifdef  BSP_USE_RT_THREAD
    if(NULL != pt)  rt_free(pt);
#else
    if(NULL != pt)  free(pt);
#endif

    return res;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: IAP_CodeCheck                                                    */
/* 功能描述: SHA1+MD5校验                                                     */
/* 输入参数: name --- 设备名称                                                */
/*           area_addr --- 检验区域的首地址                                   */
/*           size --- 检验区域的大小                                          */
/* 输出参数: 无                                                               */
/* 返 回 值: 0 = 成功                                                         */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-20              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
static int IAP_CodeCheck(const char *name, unsigned int area_addr, unsigned int size)
{
      signed int  res = 0;
    unsigned char *pt = NULL;
    unsigned int pack = 4096;
    unsigned int packs = 0;
    unsigned int remain = 0;
    unsigned int already = 0;
    struct device *dev = NULL;
    
    unsigned char sha1out[SHA1_LEN];
    SHA1_CTX sha1 = {0};
    unsigned char md5out[MD5_LEN];
    MD5_CTX  md5  = {0};

    dev = Fn_device_find(name);
    if (NULL == dev)
    {
        return -1;
    }

    while(pack)
    {
#ifdef  BSP_USE_RT_THREAD
        pt = (unsigned char *)rt_malloc(pack);
#else
        pt = (unsigned char *)malloc(pack);
#endif
        if(pt == NULL)
        {
            pack >>= 1;
        }
        else
        {
            break;
        }
    }

    if(pack == 0) 
    {
        return -2;
    }

    if(pack >= size)
    {
        pack = size;
        packs = 1;
    }
    else
    {
        remain = size % pack;
        packs  = size / pack + ((remain == 0) ? 0 : 1);
    }
    
    SHA1_Init(&sha1);
    MD5_Init(&md5);
    
    do
    {
        if (0 == (Fn_device_read(dev, area_addr, pt, pack)))
        {
            SHA1_Update(&sha1, pt, pack);
            MD5_Update(&md5, pt, pack);

            area_addr += pack;
            already   += pack;
            
            if((remain != 0) && (packs == 2))
            {
                pack = remain;
            }

            Fn_print_Progressbar("Code-Check", already, size);
        }
        else
        {
            res = -3;
            goto EXIT_LABEL;
        }
    }while(--packs > 0);
        
    SHA1_Final(&sha1, sha1out);
    MD5_Final(&md5, md5out);

    if (0 != memcmp(Sys_Param.IAP_Info.IAP_Sha1, sha1out, SHA1_LEN))
    {
        res = -4;
        goto EXIT_LABEL;
    }

    if (0 != memcmp(Sys_Param.IAP_Info.IAP_Md5, md5out, MD5_LEN))
    {
        res = -5;
        goto EXIT_LABEL;
    }

EXIT_LABEL:
#ifdef  BSP_USE_RT_THREAD
    if(NULL != pt)  rt_free(pt);
#else
    if(NULL != pt)  free(pt);
#endif

    return res;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_TimerInit                                                   */
/* 功能描述: 软件定时器初始化                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_TimerInit(void)
{   
    int i, res = 0;
    
    for (i = 0; i < sizeof(TimerTable) / sizeof(TimerTable[0]); i++)
    {
        res = Fn_timer_register(TimerTable[i].index, TimerTable[i].reload, TimerTable[i].mode, TimerTable[i].tmocb);

        if (0 == res)
        {
            if (TimerTable[i].isuse == TIMER_ENABLE)
            {
                res = Fn_timer_start(TimerTable[i].index);
                
                if (0 != res)
                {
                    return res;
                }
            }
        }
        else
        {
            return res;
        }
    }

    return res;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_FlashStateSwitch                                            */
/* 功能描述: FLASH状态切换                                                    */
/* 输入参数: ivalue --- 状态选项                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_FlashStateSwitch(FLASH_STATE ivalue)
{  
    Sys_Param.Flash_State = ivalue;
    
    switch (ivalue)
    {
    case FLASH_CHECK:
        dbug("Eflash stutes switch: FLASH_CHECK\r\n");
        break;

    case FLASH_ERASE:
        dbug("Eflash stutes switch: FLASH_ERASE\r\n");
        break;

    case FLASH_CARRY:
        dbug("Eflash stutes switch: FLASH_CARRY\r\n");
        break;

    case FLASH_SURE:
        dbug("Eflash stutes switch: FLASH_SURE\r\n");
        break;

    case FLASH_OUT:
        dbug("Eflash stutes switch: FLASH_OUT\r\n");
        break;

    default:
        dbug("Eflash stutes switch: FLASH_UNK\r\n");
        COMM_SystemStateSwitch(SYS_ERR);
        break;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SystemStateSwitch                                           */
/* 功能描述: 系统状态切换                                                     */
/* 输入参数: ivalue --- 状态选项                                              */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SystemStateSwitch(SYS_STATE ivalue)
{   
    Sys_Param.Sys_State = ivalue;

    switch (ivalue)
    {
    case SYS_INIT:
        dbug("System stutes switch: SYS_INIT\r\n");
        break;

    case SYS_CMD:
        dbug("System stutes switch: SYS_CMD\r\n");
        break;
    
    case SYS_IAP:
        dbug("System stutes switch: SYS_IAP\r\n");
        COMM_FlashStateSwitch(FLASH_CHECK);
        break;

    case SYS_LOAD:
        dbug("System stutes switch: SYS_LOAD\r\n");
        break;

    default:
        dbug("System stutes switch: SYS_ERR\r\n");
        break;
    }

    return -1;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_GetBootInfo                                                 */
/* 功能描述: 获取BOOT标识信息                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_GetBootInfo(void)
{
    int    hwver, fwver;  
    char   ver[16] = {0};

    hwver = Fn_atoi_version(BOOT_HW_VERSION, '.');
    fwver = Fn_atoi_version(BOOT_FW_VERSION, '.');
    
    if (0 != IAP_OperBootInfo((unsigned char *)&Sys_Param.BOOT_Info, sizeof(Sys_Param.BOOT_Info), READ))
    {
        return -1;
    }

    Fn_itoa_version(Sys_Param.BOOT_Info.hwVer, ver, '.');
    dbug("->boot hwVersion %s\r\n", ver);
    memset(ver, 0, sizeof(ver));

    Fn_itoa_version(Sys_Param.BOOT_Info.fwVer, ver, '.');
    dbug("->boot fwVersion %s\r\n", ver);
    memset(ver, 0, sizeof(ver));

    Fn_itoa_version(Sys_Param.BOOT_Info.appVer, ver, '.');
    dbug("->boot appVersion %s\r\n\r\n", ver);
    memset(ver, 0, sizeof(ver));
    
    if ((hwver != Sys_Param.BOOT_Info.hwVer) || (fwver != Sys_Param.BOOT_Info.fwVer))
    {
        Sys_Param.BOOT_Info.hwVer = hwver;
        Sys_Param.BOOT_Info.fwVer = fwver;

        if (0 != IAP_OperBootInfo((unsigned char *)&Sys_Param.BOOT_Info, sizeof(Sys_Param.BOOT_Info), WRITE))
        {
            return -2;
        }
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_GetUpdateInfo                                               */
/* 功能描述: 获取更新标识信息                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_GetUpdateInfo(void)
{ 
    char ver[64] = {0};
    
    if (0 != IAP_OperUpdateInfo((unsigned char *)&Sys_Param.IAP_Info, sizeof(Sys_Param.IAP_Info), READ))
    {
        return -1;
    }

    if (APP_UPGRADE_FLAG == Sys_Param.IAP_Info.IAP_Flag)
    {
        dbug("## IAP file name: %s\r\n",  Sys_Param.IAP_Info.IAP_Name);
        dbug("## IAP file lens: %dB\r\n", Sys_Param.IAP_Info.IAP_Len);

        Fn_itoa_version(Sys_Param.IAP_Info.IAP_Version, ver, '.');
        dbug("## IAP file vers: %s\r\n", ver);

        Fn_HexToStr(Sys_Param.IAP_Info.IAP_Md5,  MD5_LEN, ver);
        dbug("## IAP file md5 : %s\r\n", ver);

        Fn_HexToStr(Sys_Param.IAP_Info.IAP_Sha1, SHA1_LEN, ver);
        dbug("## IAP file sha1: %s\r\n", ver);
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_ClearUpdateFlag                                             */
/* 功能描述: 清除升级状态标志位,保存最新版本号                               */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_ClearUpdateFlag(void)
{
    Sys_Param.BOOT_Info.appVer = Sys_Param.IAP_Info.IAP_Version;
    IAP_OperBootInfo((unsigned char *)&Sys_Param.BOOT_Info, sizeof(Sys_Param.BOOT_Info), WRITE);
    
    memset((unsigned char *)&Sys_Param.IAP_Info, 0xFF, sizeof(Sys_Param.IAP_Info));
    return IAP_OperUpdateInfo((unsigned char *)&Sys_Param.IAP_Info, sizeof(Sys_Param.IAP_Info), WRITE);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SetUpdateInfo                                               */
/* 功能描述: 设置升级信息                                                    */
/* 输入参数: iap --- 升级信息参数结构体                                      */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SetUpdateInfo(struct _iap_param *iap)
{
    return IAP_OperUpdateInfo((unsigned char *)iap, sizeof(IAP_PARAM), WRITE);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_CmdmodeFlag                                                 */
/* 功能描述: 检测串口是否收到进入命令模式的字符串"#####"                     */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 收到返回0, !0未收到                                             */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-28              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_CmdmodeFlag(void)
{
    unsigned char *buf = NULL;
    struct device *dev = NULL;

    dev = Fn_device_find("console");
    if (NULL == dev)  return -1;
    
    buf = (unsigned char *)malloc(128);
    if (NULL == buf)  return -2;

    Fn_device_read(dev, 0, buf, 128);
    if (!strstr((const char *)buf, _boot_cmd))
    {
        if (NULL != buf)  free(buf);
        return -3;
    }
    
    if (NULL != buf)  free(buf);
    return 0;    
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SysInitHandle                                               */
/* 功能描述: 系统初始化处理函数                                               */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SysInitHandle(void)
{
    int res1, res2;
    struct timer  *tim1, *tim2;
    
    COMM_TimerInit();
    
    tim1 = Fn_timer_find(TIMER_LOAD);
    tim2 = Fn_timer_find(TIMER_WAIT);
    
    if ((NULL == tim1) || (NULL == tim2))
    {
        return COMM_SystemStateSwitch(SYS_ERR);
    }

    res1 = Fn_timer_start(TIMER_WAIT);
    res2 = Fn_timer_start(TIMER_LOAD);

    if ((0 != res1) || (0 != res2))
    {
        return COMM_SystemStateSwitch(SYS_ERR);
    }
    
    dbug("The system is loading, please wait  ");
    
    while (tim2->current != 0)
    {
#ifdef  BSP_USE_RT_THREAD
        rt_thread_delay(1);
#endif
    }
    
    dbug("\r\n\r\n");

    if (0 != COMM_GetBootInfo())
    {
        return COMM_SystemStateSwitch(SYS_ERR);
    }

    if (0 == COMM_CmdmodeFlag())
    {
        return COMM_SystemStateSwitch(SYS_CMD);
    }
    else
    {
        return COMM_SystemStateSwitch(SYS_IAP);
    }
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SysCmdHandle                                                */
/* 功能描述: 命令行处理模式                                                   */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SysCmdHandle(void)
{
    return cmd_task();
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SysIAPHandle                                                */
/* 功能描述: IAP搬运Code功能的具体实现                                        */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SysIAPHandle(void)
{
    struct device *dev;
    
#ifdef  BSP_MCU_STM32F4    
    int erase_cmd = STM_FLASH_ERASE_SECTOR;
    int erase_area[2] = {APP_SECTOR_START, APP_SECTOR_END};
#endif
#ifdef  BSP_MCU_STM32F1
    int erase_cmd = STM_FLASH_ERASE_PAGE;
    int erase_area[2] = {APP_FLASH_START, APP_FLASH_END};
#endif
    
    dev = Fn_device_find(INSIDE_FLASH);
    if (NULL == dev)
    {
        return COMM_SystemStateSwitch(SYS_ERR);
    }
    
    switch (Sys_Param.Flash_State)
    {
    case FLASH_CHECK:
        if (0 == COMM_GetUpdateInfo())
        {
            if (APP_UPGRADE_FLAG == Sys_Param.IAP_Info.IAP_Flag)
            {   
                dbug("Eflash checks result: PASS\r\n");
                COMM_FlashStateSwitch(FLASH_ERASE);
            }
            else
            {
                COMM_FlashStateSwitch(FLASH_OUT);
                return COMM_SystemStateSwitch(SYS_LOAD);
            }
        }
        else
        {
            dbug("Eflash checks result: FAIL\r\n");
            COMM_FlashStateSwitch(FLASH_OUT);
            return COMM_SystemStateSwitch(SYS_ERR);
        }
        break;

    case FLASH_ERASE:
        if (0 == Fn_device_control(dev, erase_cmd, &erase_area[0]))
        {
            dbug("Eflash erases result: PASS\r\n");
            COMM_FlashStateSwitch(FLASH_CARRY);
        }
        else
        {
            dbug("Eflash erases result: FAIL\r\n");
            COMM_FlashStateSwitch(FLASH_OUT);
        }
        break;

    case FLASH_CARRY:
        if (0 == IAP_CodeCarry(EFLASH_IAP_START, APP_FLASH_START, Sys_Param.IAP_Info.IAP_Len, W25Q_TO_STM))
        {
            dbug("Eflash carrys result: PASS\r\n");
            COMM_FlashStateSwitch(FLASH_SURE);
        }
        else
        {
            dbug("Eflash carrys result: FAIL\r\n");
            COMM_FlashStateSwitch(FLASH_OUT);
        }
        break;

    case FLASH_SURE:
        if (0 == IAP_CodeCheck(INSIDE_FLASH, APP_FLASH_START, Sys_Param.IAP_Info.IAP_Len))
        {
            dbug("Iflash checks result: PASS\r\n");
            COMM_ClearUpdateFlag();
            COMM_FlashStateSwitch(FLASH_OUT);
        }
        else
        {
            dbug("Iflash checks result: FAIL\r\n");
            COMM_FlashStateSwitch(FLASH_OUT);
        }
        break;

    default:
        COMM_SystemStateSwitch(SYS_CMD);
        break;
    }

    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SysLoadHandle                                               */
/* 功能描述: 程序跳转模式                                                     */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SysLoadHandle(void)
{
    IAP_JumpToApp(APP_FLASH_START);
    dbug("System stutes switch: JUMP_FAIL\r\n");
    return COMM_SystemStateSwitch(SYS_CMD);
}

/*<FUNC+>**********************************************************************/
/* 函数名称: COMM_SysErrHandle                                                */
/* 功能描述: 程序错误模式                                                     */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2020-11-19              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int COMM_SysErrHandle(void)
{
    NVIC_SystemReset();
    return 0;
}

/*<FUNC+>**********************************************************************/
/* 函数名称: boot_application                                                 */
/* 功能描述: boot应用入口函数                                                 */
/* 输入参数: 无                                                               */
/* 输出参数: 无                                                               */
/* 返 回 值: int                                                              */
/* 操作流程:                                                                  */
/* 其它说明: 无                                                               */
/* 修改记录:                                                                  */
/* -------------------------------------------------------------------------- */
/*     2021-01-27              v 1.0.0       Leonard       创建函数           */
/*<FUNC->**********************************************************************/
int boot_application(void)
{
    while (1)
    {
        switch(Sys_Param.Sys_State)
        {
        case SYS_INIT:
            COMM_SysInitHandle();
            break;

        case SYS_CMD:
            COMM_SysCmdHandle();
            break;

        case SYS_IAP:
            COMM_SysIAPHandle();
            break;

        case SYS_LOAD:
            COMM_SysLoadHandle();
            break;

        default:
            COMM_SysErrHandle();
            break;
        }
    }
}
INIT_APP_EXPORT(boot_application);

