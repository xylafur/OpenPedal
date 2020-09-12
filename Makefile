###############################################################################
# Global Build Parameters
#  We want all of these flags to be passed to KESL
export CC=arm-none-eabi-gcc
export OBJCOPY=arm-none-eabi-objcopy
export OBJDUMP=arm-none-eabi-objdump
export GDB=arm-none-eabi-gdb
export SIZE=arm-none-eabi-size

export CFLAGS  = -Wall -g -std=c99 -Os
export CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
export CFLAGS += -ffunction-sections -fdata-sections
export CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map
export CFLAGS += -Wdouble-promotion -Werror -Wundef
export CFLAGS += --specs=nano.specs

###############################################################################
# Open Pedal source files and local includes
SRCS = src/main.c
SRCS += src/cli.c

SRCS += src/transform/transform.c

SRCS += src/output/output.c
SRCS += src/output/pwm.c

PROJ_NAME := open_pedal

CFLAGS +=   -I ./src/include/

OPENOCD_BOARD_DIR := /usr/share/openocd/scripts/board
OPENOCD_PROC_FILE := extra/stm32f0-openocd.cfg

###############################################################################
# Everything that comes from KESL
#  Linker Scripts, MPU Files (startup, periph map etc), the stdperiph lib we
#  use
KESL := $(CURDIR)/KESL

MPU_BASE := $(KESL)/mpu/stm32f0
CORE_BASE := $(KESL)/core
LIB_BASE := $(KESL)/lib/stm32f0

MPU_INCLUDE := $(MPU_BASE)/include
CORE_INCLUDE := $(CORE_BASE)/include
LIB_INCLUDE := $(LIB_BASE)
STD_PERIPH_INCLUDE := $(LIB_BASE)/STM32F0xx_StdPeriph_Driver/include
CPU_INCLUDE := $(KESL)/cpu

KESL_INCLUDE = -I$(MPU_INCLUDE) -I$(CORE_INCLUDE) -I$(LIB_INCLUDE) \
			   -I$(CPU_INCLUDE) -I$(STD_PERIPH_INCLUDE)

MPU_OBJS = $(shell find $(MPU_BASE)/objs/ -name *.o)
CORE_OBJS = $(shell find $(CORE_BASE)/objs/ -name *.o)
LIB_OBJS = $(shell find $(LIB_BASE)/objs/ -name *.o)

LINKER_DIR := $(MPU_BASE)/ldscripts
LDSCRIPT_INC := $(LINKER_DIR)
LINKER_SCRIPT := $(LINKER_DIR)/stm32f0.ld

###############################################################################
# Include the objects for this project, as well as the objects that KESL creates

KESL_OBJS = $(MPU_OBJS) $(CORE_OBJS) $(LIB_OBJS)
OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: proj kesl
all: proj

kesl:
	$(MAKE) --directory=KESL

proj: 	$(PROJ_NAME).bin

$(SRCS): kesl

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $(KESL_INCLUDE) $(KESL_OBJS) $^ -o $@  -L$(LDSCRIPT_INC) -T$(LINKER_SCRIPT)

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
	@find ./ -name '*~' | xargs rm -f
	@rm -f *.o
	@rm -f $(PROJ_NAME).elf
	@rm -f $(PROJ_NAME).hex
	@rm -f $(PROJ_NAME).bin
	@rm -f $(PROJ_NAME).map
	@rm -f $(PROJ_NAME).lst
	@$(MAKE) --directory=KESL clean
