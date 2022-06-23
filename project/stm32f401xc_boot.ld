/******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32F401VCTx Device with
**                256KByte FLASH, 64KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
**  (c)Copyright Ac6.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Ac6 permit registered System Workbench for MCU users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the System Workbench for MCU toolchain.
**
******************************************************************************/

/* Entry Point  程序入口——程序将从Reset_Handler开始执行，而该函数定义在stm32fxxx.s启动文件中 */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20010000;       /* end of SRAM  堆栈末尾 = SRAM起始地址 + SRAM空间大小 */

/* Generate a link error if heap and stack don't fit into RAM   c程序所必须的堆、栈空间大小定义 */
_Min_Heap_Size = 0x100;     /* required amount of heap  */
_Min_Stack_Size = 0x400;    /* required amount of stack */

/* Specify the memory areas   单片机内部存储空间 SRAM ROM CCM 起始位置和大小的声明 */
/* CCM 参考 https://club.rt-thread.org/ask/article/1f31bacada2c883b.html, DMA禁止访问CCM, DMA只能访问SRAM */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
    RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 64K
    /* CCM (xrw)  : ORIGIN = 0x10000000, LENGTH = 64K */
}

/* Define output sections */
SECTIONS
{
    /* The startup code goes first into FLASH */
    /* 中断向量表定义于 .s启动文件中，应位于Flash的最前端，也就是从0x8000000的位置开始 */
    .isr_vector :
    {
        . = ALIGN(4);           /* 将本地计数器 . 强制按4B对齐 */
        KEEP(*(.isr_vector))    /* 将中断向量的内容链接到该地址, 并将本地计数器 . 的值自动递增 */
        . = ALIGN(4);
    } >FLASH

    /* The program code and other data goes into FLASH  .text 对应程序的可执行代码 */
    .text :
    {
        . = ALIGN(4);
        *(.text)            /* .text sections (code) */
        *(.text*)           /* .text* sections (code) */
        *(.glue_7)          /* glue arm to thumb code */
        *(.glue_7t)         /* glue thumb to arm code */
        *(.eh_frame)
        *(.vfp11_veneer)    /* add by me */
        *(.v4_bx)           /* add by me */
        *(.iplt)            /* add by me */
                            /* KEEP() 的作用是当启用连接器的--gc-sections垃圾回收选项时，这部分不能被回收 */
                            /* 参考链接 https://blog.csdn.net/wangbinyantai/article/details/79001303 */
        KEEP (*(.init))     /* *() 的作用就是把括号内的内容写入 }外 >指向的区域，KEEP()相当于是又多了一层操作，理解为不回收 */
        KEEP (*(.fini))

        . = ALIGN(4);       /* 把当前计数器 . 的值向下4字节对齐取整 */
                            /* _etext是链接器的预定义变量，代表程序正文段结束的第一个地址 */
        _etext = .;         /* define a global symbols at end of code 定义一个全局变量 _etext，并把 . 的值赋给他 */
    } >FLASH

    /* Constant data goes into FLASH  .rodata 代表程序中使用的常量表格数据等 */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)          /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)         /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
    } >FLASH

    .ARM.extab :
    { 
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* .preinit_array和.init_array部分包含指向将在初始化时调用的函数指针数组 */
    /* 参考：https://stackoverflow.com/questions/40532180/understanding-the-linkerscript-for-an-arm-cortex-m-microcontroller */
    /* PROVIDE_HIDDEN(For ELF targeted ports, the symbol will be hidden and won’t be exported 符号在目标文件中被定义但不会被导出 */
    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    /* 以变量 _sidata 存储 .data段在rom中的存储起始地址, 并将 .data段中的数据放入以此地址开始的FLASH中，共放入.data段的长度 */
    /* used by the startup to initialize data   */
    _sidata = LOADADDR(.data);

    /* 因为前面都是 >FLASH, 所以第一个. 的初始值就是FLASH的初始值(ORIGIN = 0x08000000),其后继续 >FLASH会自动调整 .的值 */
    /* 下面开始 >RAM,则 .的值不会在前面的基础上继续调整，而是变为RAM段的起始地址(ORIGIN = 0x20000000),其后继续 >RAM的内容会自动调整 .的值 */

    /* Initialized data sections goes into RAM, load LMA copy after code */
    /* .data对应初始化了的全局变量，编译后将位于可执行文件中，由启动代码负责加载到数据区中（在单片机中这部分数据会存于flash中，需要由启动代码把这部分内容拷贝到ram中）*/
    .data :
    {
        . = ALIGN(4);
        _sdata = .;         /* 申明全局符号 _sdata 同时将本地计数器 . 赋值给符号; _sdata == 0x20000000 即RAM段的ORIGIN */
        __data_start__ = _sdata;

        *(.data)            /* .data sections */
        *(.data*)           /* .data* sections */

        . = ALIGN(4);
        _edata = .;         /* _edata == _sdata + .data段的长度 */
        __data_end__ = _edata;
    } >RAM  AT>FLASH

    /* .bss段是 没有初始值/初始值为0 的全局变量，由启动代码把这部分内容全初始化为0 */
    .bss :
    {
        . = ALIGN(4);
                            /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;          /* define a global symbol at bss start   不考虑对齐时 _sbss == _edata */

        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)           /* COMMON数据段仅在.o文件中存在，当代码中的全局变量没有赋初值时存储在该数据段中 */
                            /* 参考 https://stackoverflow.com/questions/16835716/bss-vs-common-what-goes-where */

        . = ALIGN(4);
        _ebss = .;          /* define a global symbol at bss end;  _ebss == _sbss + .bss段的长度 */
        __bss_end__ = _ebss;
    } >RAM

    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
        . = ALIGN(8);

        PROVIDE ( end = . );    /* 同PROVIDE_HIDDEN的作用类似，定义内部变量而不导出 */
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM

    /* Remove information from the standard libraries  从标准库中删除信息 */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 :
    {
        *(.ARM.attributes)
    }
}

