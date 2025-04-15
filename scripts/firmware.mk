FIRMWARE_MK_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

OUT := $(abspath $(FIRMWARE_MK_DIR)../out)/

CC := riscv64-unknown-elf-gcc
OBJCOPY := riscv64-unknown-elf-objcopy
OBJDUMP := riscv64-unknown-elf-objdump
ICEPROG := iceprog
HEXDUMP := hexdump
HEXDUMP_ARGS := -v -e '1/4 "%08x" "\n"'
ARCH := rv32im
ABI := ilp32
PICOLIBC_INCLUDE := /usr/lib/picolibc/riscv64-unknown-elf/include
PICOLIBC_LIB := /usr/lib/picolibc/riscv64-unknown-elf/lib
GCC_LIB := /usr/lib/gcc/riscv64-unknown-elf/10.2.0

CFLAGS = -Wall \
		 -Werror \
		 -Os \
		 -march=$(ARCH) \
		 -mabi=$(ABI) \
		 -ffreestanding \
		 -flto \
		 -fomit-frame-pointer \
		 -nostartfiles \
		 -nodefaultlibs \
		 -nostdlib \
		 -I$(PICOLIBC_INCLUDE) \
		 -I$(abspath $(FIRMWARE_MK_DIR)../hal) \
		 -DFIRMWARE

HAL_HEADERS := $(abspath $(FIRMWARE_MK_DIR)../hal/up5k_soc.h) \
	      	   $(abspath $(FIRMWARE_MK_DIR)../hal/acia.h) \
		  	   $(abspath $(FIRMWARE_MK_DIR)../hal/spi.h) \
		  	   $(abspath $(FIRMWARE_MK_DIR)../hal/flash.h) \
		  	   $(abspath $(FIRMWARE_MK_DIR)../hal/clkcnt.h) \
		  	   $(abspath $(FIRMWARE_MK_DIR)../hal/ili9341.h) \
		  	   $(abspath $(FIRMWARE_MK_DIR)../hal/i2c.h) \
		  	   $(abspath $(FIRMWARE_MK_DIR)../hal/printf.h)

HAL_SOURCES := $(abspath $(FIRMWARE_MK_DIR)../hal/acia.c) \
			   $(abspath $(FIRMWARE_MK_DIR)../hal/spi.c) \
			   $(abspath $(FIRMWARE_MK_DIR)../hal/flash.c) \
			   $(abspath $(FIRMWARE_MK_DIR)../hal/clkcnt.c) \
			   $(abspath $(FIRMWARE_MK_DIR)../hal/ili9341.c) \
			   $(abspath $(FIRMWARE_MK_DIR)../hal/i2c.c) \
			   $(abspath $(FIRMWARE_MK_DIR)../hal/printf.c)

START_SCRIPT := $(abspath $(FIRMWARE_MK_DIR)../startup/start.S)

all: $(OUT)$(FIRMWARE).bin

$(OUT)$(FIRMWARE).elf: $(LDSCRIPT) $(START_SCRIPT) $(HAL_HEADERS) $(HAL_SOURCES) $(HEADERS) $(SOURCES)
	@mkdir -p $(OUT);	
	$(CC) $(CFLAGS) \
		-Wl,-Bstatic,-T,$(LDSCRIPT),--strip-debug,-L$(PICOLIBC_LIB)/$(ARCH)/$(ABI),-L$(GCC_LIB),--whole-archive,-lc,--no-whole-archive,-lgcc,--gc-sections \
		-o $(OUT)$(FIRMWARE).elf \
		$(START_SCRIPT) \
		$(HAL_SOURCES) \
		$(SOURCES) || exit 1

disassemble: $(OUT)$(FIRMWARE).elf
	@mkdir -p $(OUT);
	$(OBJDUMP) -d $(OUT)$(FIRMWARE).elf > $(OUT)$(FIRMWARE).dis

$(OUT)$(FIRMWARE).bin: $(OUT)$(FIRMWARE).elf
	@mkdir -p $(OUT);
	$(OBJCOPY) -O binary $(OUT)$(FIRMWARE).elf $(OUT)$(FIRMWARE).bin

$(OUT)$(FIRMWARE).hex: $(OUT)$(FIRMWARE).bin $(EXTRA_DEPS)
	@mkdir -p $(OUT);
	$(HEXDUMP) $(HEXDUMP_ARGS) $(OUT)$(FIRMWARE).bin > $(OUT)$(FIRMWARE).hex

clean:
	rm -f $(OUT)*.bin $(OUT)*.hex $(OUT)*.elf $(OUT)*.dis
