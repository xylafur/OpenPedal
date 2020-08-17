export CC=arm-none-eabi-gcc
export OBJCOPY=arm-none-eabi-objcopy
export OBJDUMP=arm-none-eabi-objdump
export GDB=arm-none-eabi-gdb
export SIZE=arm-none-eabi-size

# Main Project
SRCS = main.c
SRCS += cli.c

SRCS += transform/transform.c

SRCS += output/output.c
SRCS += output/pwm.c


PROJ_NAME := open_pedal

KESL := $(CURDIR)/KESL
LDSCRIPT_INC := $(KESL)/mpu/stm32f0/ldscripts

OPENOCD_BOARD_DIR := /usr/share/openocd/scripts/board
OPENOCD_PROC_FILE := =extra/stm32f0-openocd.cfg

export CFLAGS  = -Wall -g -std=c99 -Os
export CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
export CFLAGS += -ffunction-sections -fdata-sections
export CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map
export CFLAGS += -Wdouble-promotion -Werror -Wundef
export CFLAGS += --specs=nano.specs

vpath %.c src
vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

CFLAGS +=   -I ./src/include/

MPU_BASE := $(KESL)/mpu
CORE_BASE := $(KESL)/core
LIB_BASE := $(KESL)/lib/stm32f0

MPU_INCLUDE := $(MPU_BASE)/stm32f0/include
CORE_INCLUDE := $(CORE_BASE)/include
LIB_INCLUDE := $(LIB_BASE)/STM32F0xx_StdPeriph_Driver/include
CPU_INCLUDE := $(KESL)/cpu

KESL_INCLUDE = -I$(MPU_INCLUDE) -I$(CORE_INCLUDE) -I$(LIB_INCLUDE) -I$(CPU_INCLUDE)

MPU_OBJS = $(MPU_BASE)/objs/%.o
CORE_OBJS = $(CORE_BASE)/objs/%.o
LIB_OBJS = $(LIB_BASE)/objs/%.o

KESL_OBJS = $(MPU_OBJS) $(CORE_OBJS) $(LIB_OBJS)


OBJS = $(SRCS:.c=.o)

# Kesl Core
SRCS += kesl/core/logger.c
SRCS += kesl/core/syscalls.c
SRCS += kesl/core/kcb.c

# Kesl Library / System Config / Startup
SRCS += kesl/mpu/stm32f0/system_stm32f0xx.c
SRCS += ./kesl/mpu/stm32f0/startup_stm32f0xx.s




###################################################

.PHONY: lib proj kesl

all: kesl proj

kesl:
	$(MAKE) --directory=KESL

proj: 	$(PROJ_NAME).bin

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $(KESL_INCLUDE) $(KESL_OBJS) $^ -o $@ -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

$(PROJ_NAME).bin: $(PROJ_NAME).elf
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(OBJDUMP) -St $(PROJ_NAME).elf >$(PROJ_NAME).lst
	$(SIZE) $(PROJ_NAME).elf

program: $(PROJ_NAME).bin
	openocd -f $(OPENOCD_BOARD_DIR)/stm32f0discovery.cfg -f $(OPENOCD_PROC_FILE) -c "stm_flash `pwd`/$(PROJ_NAME).bin" -c shutdown

debug: program
	$(GDB) -x extra/gdb_cmds $(PROJ_NAME).elf

clean:
	find ./ -name '*~' | xargs rm -f
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).lst
	$(MAKE) --directory=KESL clean

reallyclean: clean
	$(MAKE) -C $(STD_PERIPH_LIB) clean
