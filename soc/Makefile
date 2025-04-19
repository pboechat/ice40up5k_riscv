OUT := $(abspath ../out)/

SOURCES := up5k.v \
           soc.v \
		   spram_16kx32.v \
		   spram_32kx32.v \
	       acia.v \
		   acia_rx.v \
		   acia_tx.v \
	       wb_bus.v \
		   wb_master.v \
	       ../picorv32/picorv32.v 

# firmware
FIRMWARE ?= demo

# rtl
PIN_DEF := up5k.pcf
SDC := up5k.sdc
DEVICE := up5k
PACKAGE := sg48

YOSYS := yosys
NEXTPNR := nextpnr-ice40
ICEPACK := icepack
ICETIME := icetime
ICEPROG := iceprog
ICEBRAM := icebram
CDCPROG := cdc_prog
VERILATOR := verilator
TECH_LIB := cells_sim.v

all: $(OUT)$(FIRMWARE)_up5k_soc.bin

$(OUT)placeholder_firmware.hex:
	@mkdir -p $(OUT);
	$(ICEBRAM) -g 32 2048 > $(OUT)placeholder_firmware.hex

$(OUT)up5k_soc.json: $(SOURCES) $(OUT)placeholder_firmware.hex
	$(YOSYS) -p 'synth_ice40 -dsp -top up5k -json $(OUT)up5k_soc.json' $(SOURCES) || exit 1

$(OUT)up5k_soc.asc: $(OUT)up5k_soc.json $(PIN_DEF)
	@mkdir -p $(OUT);
	$(NEXTPNR) --pre-pack $(SDC) --placer heap --timing-allow-fail --$(DEVICE) --package $(PACKAGE) --json $(OUT)up5k_soc.json --pcf $(PIN_DEF) --asc $(OUT)up5k_soc.asc || exit 1

$(FIRMWARE).hex:
	$(MAKE) -C ../$(FIRMWARE)/ all || exit 1

$(OUT)$(FIRMWARE)_up5k_soc.bin: $(OUT)up5k_soc.asc $(FIRMWARE).hex
	@mkdir -p $(OUT);
	$(ICEBRAM) $(OUT)placeholder_firmware.hex $(OUT)$(FIRMWARE).hex < $(OUT)up5k_soc.asc > $(OUT)temp.asc
	$(ICEPACK) $(OUT)temp.asc $(OUT)$(FIRMWARE)_up5k_soc.bin

$(OUT)up5k_soc.rpt: $(OUT)up5k_soc.asc
	@mkdir -p $(OUT);
	$(ICETIME) -d $(DEVICE) -mtr $(OUT)up5k_soc.rpt $(OUT)up5k_soc.asc

prog: $(OUT)$(FIRMWARE)_up5k_soc.bin
	@mkdir -p $(OUT);
	$(CDCPROG) -p /dev/ttyACM0 $(OUT)$(FIRMWARE)_up5k_soc.bin

recode:
	rm -f $(OUT)$(FIRMWARE).hex $(OUT)$(FIRMWARE)_up5k_soc.bin
	$(MAKE) prog

flash: $(OUT)$(FIRMWARE)_up5k_soc.bin
	$(CDCPROG) -w -p /dev/ttyACM0 $(OUT)$(FIRMWARE)_up5k_soc.bin

lint: $(SOURCES)
	$(VERILATOR) --lint-only -Wall --top-module up5k $(TECH_LIB) $(SOURCES)

clean:
	$(MAKE) -C ../$(FIRMWARE)/ clean
	rm -f $(OUT)*.json $(OUT)*.asc $(OUT)*.rpt $(OUT)*.bin

.PHONY: all prog flash recode lint clean
