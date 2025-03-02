OUT = ../out

CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy
OBJDUMP = riscv64-unknown-elf-objdump
ICEPROG = iceprog
HEXDUMP = hexdump
HEXDUMP_ARGS = -v -e '1/4 "%08x" "\n"'
ARCH = rv32i
ABI = ilp32
PICOLIBC_INCLUDE = /usr/lib/picolibc/riscv64-unknown-elf/include
PICOLIBC_LIB = /usr/lib/picolibc/riscv64-unknown-elf/lib
GCC_LIB = /usr/lib/gcc/riscv64-unknown-elf/10.2.0

CFLAGS=-Wall -Os -march=$(ARCH) -mabi=$(ABI) -ffreestanding -flto -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -I$(PICOLIBC_INCLUDE)

HEADER = up5k_riscv.h acia.h spi.h flash.h clkcnt.h ili9341.h i2c.h printf.h

SOURCES = start.s main.c acia.c spi.c flash.c clkcnt.c ili9341.c i2c.c printf.c

all: firmware.hex

%.elf: firmware.ld $(HEADERS) $(SOURCES)
	@mkdir -p $(OUT);	
	$(CC) $(CFLAGS) -Wl,-Bstatic,-T,firmware.ld,--strip-debug,-L$(PICOLIBC_LIB)/$(ARCH)/$(ABI),-L$(GCC_LIB),--whole-archive,-lc,--no-whole-archive,-lgcc,--gc-sections -o $(OUT)/$@ $(SOURCES)

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

.PHONY: all disassemble clean