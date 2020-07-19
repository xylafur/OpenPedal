CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
GDB=arm-none-eabi-gdb
SIZE=arm-none-eabi-size

# Main Project
SRCS = main.c
SRCS += cli.c
SRCS += transform/transform.c
SRCS += output/pwm.c

# Kesl Core
SRCS += kesl/core/logger.c
SRCS += kesl/core/syscalls.c
SRCS += kesl/core/kcb.c

# Kesl Library / System Config / Startup
SRCS += kesl/mpu/stm32f0/system_stm32f0xx.c
SRCS += ./kesl/mpu/stm32f0/startup_stm32f0xx.s

PROJ_NAME=open_pedal

KESL=./kesl

STD_PERIPH_LIB=$(KESL)/lib/stm32f0

LDSCRIPT_INC=$(KESL)/mpu/stm32f0/ldscripts

OPENOCD_BOARD_DIR=/usr/share/openocd/scripts/board
OPENOCD_PROC_FILE=extra/stm32f0-openocd.cfg

CFLAGS  = -Wall -g -std=c99 -Os
#CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map
CFLAGS += -Wdouble-promotion -Werror -Wundef
CFLAGS += --specs=nano.specs

vpath %.c src
vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

CFLAGS +=   -I $(KESL)/lib/
CFLAGS +=   -I $(STD_PERIPH_LIB)
CFLAGS +=   -I $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/include/
CFLAGS +=   -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/include/
CFLAGS +=   -I $(KESL)/lib/stm32f0/CMSIS/include/
CFLAGS +=   -I $(KESL)/include/
CFLAGS +=   -I ./src/include/


#CFLAGS += -I src/include
#CFLAGS += -I kesl/include
#CFLAGS += -I $(STD_PERIPH_LIB)
#CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include
#CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Include
#CFLAGS += -I $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc
#CFLAGS += -I $(STD_PERIPH_LIB)/stm32f0xx_conf.h

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C $(STD_PERIPH_LIB)

proj: 	$(PROJ_NAME).bin

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

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

reallyclean: clean
	$(MAKE) -C $(STD_PERIPH_LIB) clean
