#
# Copyright (C) lance. 2022. All rights reserved
#

SERIES   := STM32F4xx
BOARD    := STM32F401xC
CURDIR   := $(shell pwd)
TARGET   := $(BOARD)_boot
OBJPATH  := $(CURDIR)/object

USERTOS  := FreeRTOS
USBROLE  := Device
USBCLASS := CDC

#
# Compiler options
#

DEBUG = -g
OPTIMIZE = -O0
WARNING = -W -Wall -Werror

#
# Processor options
#

MCU_ARCH = cortex-m4
FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DARM_MATH_CM4
PREDEFINES = -D$(BOARD) -DUSE_HAL_DRIVER -DHSE_VALUE=25000000 -DRUN_FROM_FLASH=1

#
# CMSIS source file and start-up head file
#

START_ASM := $(CURDIR)/library/CMSIS/Device/ST/$(SERIES)/Source/Templates/gcc/startup_$(shell echo $(BOARD) | tr A-Z a-z).s
CMSIS_SRC += $(CURDIR)/library/CMSIS/Device/ST/$(SERIES)/Source/Templates/system_$(shell echo $(SERIES) | tr A-Z a-z).c
CMSIS_SRC += $(CURDIR)/project/$(shell echo $(SERIES) | tr A-Z a-z)_hal_msp.c
CMSIS_SRC += $(CURDIR)/project/$(shell echo $(SERIES) | tr A-Z a-z)_it.c
CMSIS_SRC += $(CURDIR)/project/main.c

#
# HAL source file
#

ALL_HAL_SRC += $(wildcard $(CURDIR)/library/$(SERIES)_HAL_Driver/Src/*.c)
UNUSE_HAL_SRC += $(wildcard $(CURDIR)/library/$(SERIES)_HAL_Driver/Src/*_template.c)
CMHAL_SRC := $(filter-out $(UNUSE_HAL_SRC), $(ALL_HAL_SRC))

#
# Other source file
#

# OTHER_SRC += $(wildcard $(CURDIR)/library/$(USERTOS)/*.c)
# #OTHER_SRC += $(CURDIR)/library/$(USERTOS)/portable/Common/mpu_wrappers.c
# OTHER_SRC += $(CURDIR)/library/$(USERTOS)/portable/GCC/ARM_CM4F/port.c
# OTHER_SRC += $(CURDIR)/library/$(USERTOS)/portable/MemMang/heap_4.c

OTHER_SRC += $(wildcard $(CURDIR)/library/STM32_USB_$(USBROLE)_Library/Core/Src/*.c)
OTHER_SRC += $(wildcard $(CURDIR)/library/STM32_USB_$(USBROLE)_Library/Class/$(USBCLASS)/Src/*.c)

#
# Object file
#

START_OBJ = $(START_ASM:%.s=%.o)
CMSIS_OBJ = $(CMSIS_SRC:%.c=%.o)
CMHAL_OBJ = $(CMHAL_SRC:%.c=%.o)
OTHER_OBJ = $(OTHER_SRC:%.c=%.o)

#
# Cross compilation tool chain
#

CROSS_COMPILE := arm-none-eabi-
export AS   = $(CROSS_COMPILE)as
export LD   = $(CROSS_COMPILE)ld
export CC   = $(CROSS_COMPILE)gcc
export BIN  = $(CROSS_COMPILE)objcopy -Obinary
export HEX  = $(CROSS_COMPILE)objcopy -Oihex
export SIZE = $(CROSS_COMPILE)size
export GDB  = $(CROSS_COMPILE)gdb

#
# Compile parameters
#

MCUFLAGS := -mthumb -mcpu=$(MCU_ARCH) $(FPU)
SRCFLAGS := $(OPTIMIZE)  $(DEBUG)  $(MCUFLAGS)  $(WARNING)  $(PREDEFINES)
LDFLAGS  := -T $(CURDIR)/project/$(shell echo $(TARGET) | tr A-Z a-z).ld  \
            -lc -lm -lgcc  \
            -static  \
            -specs=nano.specs  \
            -specs=nosys.specs  \
            -u _printf_float  \
            -u _scanf_float  \
            -Wl,--gc-sections  \
            -Wl,--start-group  \
            -Wl,--end-group  \
            -Wl,-Map=$(OBJPATH)/$(TARGET).map  \
            -Wl,--defsym=malloc_getpagesize_P=0x80

CMSISINCFLAGS := -I $(CURDIR)/library/CMSIS/Include  \
                 -I $(CURDIR)/library/CMSIS/Device/ST/$(SERIES)/Include  \
                 -I $(CURDIR)/library/$(SERIES)_HAL_Driver/Inc/Legacy  \
                 -I $(CURDIR)/library/$(SERIES)_HAL_Driver/Inc  \
                 -I $(CURDIR)/utility/common  \
                 -I $(CURDIR)/utility/driver  \
                 -I $(CURDIR)/project

# OTHERINCFLAGS += -I $(CURDIR)/library/$(USERTOS)/include  \
#                  -I $(CURDIR)/library/$(USERTOS)/portable/GCC/ARM_CM4F
      
OTHERINCFLAGS += -I $(CURDIR)/library/STM32_USB_$(USBROLE)_Library/Core/Inc  \
                 -I $(CURDIR)/library/STM32_USB_$(USBROLE)_Library/Class/$(USBCLASS)/Inc

#
# Generate target
#

all : $(START_OBJ) $(CMSIS_OBJ) $(CMHAL_OBJ) $(OTHER_OBJ)
	$(CC)  $(SRCFLAGS) $(LDFLAGS) $(CMSISINCFLAGS) $(OTHERINCFLAGS) $^ -o $(OBJPATH)/$(TARGET).elf
	$(BIN) $(OBJPATH)/$(TARGET).elf  $(OBJPATH)/$(TARGET).bin
	$(HEX) $(OBJPATH)/$(TARGET).elf  $(OBJPATH)/$(TARGET).hex

$(START_OBJ) : %.o : %.s
	$(CC) -c $(SRCFLAGS) $(CMSISINCFLAGS) $(OTHERINCFLAGS) $^ -o $@
$(CMSIS_OBJ) : %.o : %.c
	$(CC) -c $(SRCFLAGS) $(CMSISINCFLAGS) $(OTHERINCFLAGS) $^ -o $@
$(CMHAL_OBJ) : %.o : %.c
	$(CC) -c $(SRCFLAGS) $(CMSISINCFLAGS) $(OTHERINCFLAGS) $^ -o $@
$(OTHER_OBJ) : %.o : %.c
	$(CC) -c $(SRCFLAGS) $(CMSISINCFLAGS) $(OTHERINCFLAGS) $^ -o $@

#
# Clean file
#

.PHONY :
distclean :
	$(RM) $(START_OBJ) $(CMSIS_OBJ) $(CMHAL_OBJ) $(OTHER_OBJ) $(OBJPATH)/*
clean :
	$(RM) $(START_OBJ) $(CMSIS_OBJ) $(CMHAL_OBJ) $(OTHER_OBJ)
debug :
	$(shell openocd -f project/JLinkOB.cfg)
burn :
	$(shell openocd -f project/JLinkOB.cfg  \
                    -c init  -c halt  \
	                -c "flash write_image erase $(OBJPATH)/$(TARGET).bin 0x08000000"  \
	                -c reset -c exit)


