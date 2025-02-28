OUT = ../out

CROSS = riscv64-unknown-elf-
CC = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
ICEPROG = iceprog
HEXDUMP = hexdump
HEXDUMP_ARGS = -v -e '1/4 "%08x" "\n"'

PROJ = firmware

CFLAGS=-Wall -Os -march=rv32i -mabi=ilp32 -ffreestanding -flto -fomit-frame-pointer --specs=picolibc.specs -nostartfiles

HEADER = up5k_riscv.h acia.h spi.h flash.h clkcnt.h ili9341.h i2c.h printf.h

SOURCES = start.s main.c acia.c spi.c flash.c clkcnt.c ili9341.c i2c.c printf.c

all: $(PROJ).hex

%.elf: lnk-app.lds $(HEADERS) $(SOURCES)
	@mkdir -p $(OUT);
	$(CC) $(CFLAGS) -Wl,-Bstatic,-T,lnk-app.lds,--strip-debug -o $(OUT)/$@ $(SOURCES)

disassemble: %.elf
	@mkdir -p $(OUT);
	$(OBJDUMP) -d $(OUT)/$< > $(OUT)/$(PROJ).dis

%.bin: %.elf
	@mkdir -p $(OUT);
	$(OBJCOPY) -O binary $(OUT)/$< $(OUT)/$@

%.hex: %.bin
	@mkdir -p $(OUT);
	$(HEXDUMP) $(HEXDUMP_ARGS) $(OUT)/$< > $(OUT)/$@

clean:
	rm -f $(OUT)/*.bin $(OUT)/*.hex $(OUT)/*.elf $(OUT)/*.dis

.PHONY: all disassemble clean