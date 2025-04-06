# modified from examples in icestorm distribution

OUT = ../out

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
FW_PLCHLD_HEX := plchld_firmware.hex
FW_HEX := firmware.hex

# rtl
PROJ := up5k_soc
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

all: $(PROJ).bin

$(FW_PLCHLD_HEX):
	@mkdir -p $(OUT);
	$(ICEBRAM) -g 32 2048 > $(OUT)/$(FW_PLCHLD_HEX)

%.json: $(SOURCES) $(FW_PLCHLD_HEX)
	$(YOSYS) -p 'synth_ice40 -dsp -top up5k -json $(OUT)/$@' $(SOURCES)

%.asc: %.json $(PIN_DEF)
	@mkdir -p $(OUT);
	$(NEXTPNR) --pre-pack $(SDC) --placer heap --timing-allow-fail --$(DEVICE) --package $(PACKAGE) --json $(OUT)/$< --pcf $(PIN_DEF) --asc $(OUT)/$@

$(FW_HEX):
	$(MAKE) -C ../$(FIRMWARE)/ $@

%.bin: %.asc $(FW_HEX)
	@mkdir -p $(OUT);
	$(ICEBRAM) $(OUT)/$(FW_PLCHLD_HEX) $(OUT)/$(FW_HEX) < $(OUT)/$< > $(OUT)/temp.asc
	$(ICEPACK) $(OUT)/temp.asc $(OUT)/$@

%.rpt: %.asc
	@mkdir -p $(OUT);
	$(ICETIME) -d $(DEVICE) -mtr $(OUT)/$@ $<

prog: $(PROJ).bin
	@mkdir -p $(OUT);
	$(CDCPROG) -p /dev/ttyACM0 $(OUT)/$<

recode:
	rm -f $(OUT)/$(FW_HEX) $(OUT)/$(PROJ).bin
	$(MAKE) prog

flash: $(PROJ).bin
	$(CDCPROG) -w -p /dev/ttyACM0 $(OUT)/$<

lint: $(SOURCES)
	$(VERILATOR) --lint-only -Wall --top-module up5k $(TECH_LIB) $(SOURCES)

clean:
	$(MAKE) -C ../$(FIRMWARE)/ clean
	rm -f $(OUT)/*.json $(OUT)/*.asc $(OUT)/*.rpt $(OUT)/*.bin

.PHONY: all prog flash recode lint clean
