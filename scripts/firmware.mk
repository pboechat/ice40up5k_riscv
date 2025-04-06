OUT = ../out

CC := riscv64-unknown-elf-gcc
OBJCOPY := riscv64-unknown-elf-objcopy
OBJDUMP := riscv64-unknown-elf-objdump
ICEPROG := iceprog
HEXDUMP := hexdump
HEXDUMP_ARGS := -v -e '1/4 "%08x" "\n"'
ARCH := rv32i
ABI := ilp32
PICOLIBC_INCLUDE := /usr/lib/picolibc/riscv64-unknown-elf/include
PICOLIBC_LIB := /usr/lib/picolibc/riscv64-unknown-elf/lib
GCC_LIB := /usr/lib/gcc/riscv64-unknown-elf/10.2.0

CFLAGS = -Wall -Os -march=$(ARCH) -mabi=$(ABI) -ffreestanding -flto -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -I$(PICOLIBC_INCLUDE) -I../hal

HAL_HEADERS := ../hal/up5k_riscv.h \
	      	   ../hal/acia.h \
		  	   ../hal/spi.h \
		  	   ../hal/flash.h \
		  	   ../hal/clkcnt.h \
		  	   ../hal/ili9341.h \
		  	   ../hal/i2c.h \
		  	   ../hal/printf.h

HAL_SOURCES := ../hal/acia.c \
			   ../hal/spi.c \
			   ../hal/flash.c \
			   ../hal/clkcnt.c \
			   ../hal/ili9341.c \
			   ../hal/i2c.c \
			   ../hal/printf.c

%.elf: $(LDSCRIPT) ../startup/start.S $(HAL_HEADERS) $(HAL_SOURCES) $(HEADERS) $(SOURCES)
	@mkdir -p $(OUT);	
	$(CC) $(CFLAGS) \
		-Wl,-Bstatic,-T,$(LDSCRIPT),--strip-debug,-L$(PICOLIBC_LIB)/$(ARCH)/$(ABI),-L$(GCC_LIB),--whole-archive,-lc,--no-whole-archive,-lgcc,--gc-sections \
		-o $(OUT)/$@ \
		../startup/start.S \
		$(HAL_SOURCES) \
		$(SOURCES)

disassemble: %.elf
	@mkdir -p $(OUT);
	$(OBJDUMP) -d $(OUT)/$< > $(OUT)/firmware.dis

%.bin: %.elf
	@mkdir -p $(OUT);
	$(OBJCOPY) -O binary $(OUT)/$< $(OUT)/$@

%.hex: %.bin
	@mkdir -p $(OUT);
	$(HEXDUMP) $(HEXDUMP_ARGS) $(OUT)/$< > $(OUT)/$@

clean:
	rm -f $(OUT)/*.bin $(OUT)/*.hex $(OUT)/*.elf $(OUT)/*.dis