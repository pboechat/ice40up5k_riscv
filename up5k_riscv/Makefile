# Modified from examples in icestorm distribution
# 01-16-18 E. Brombaugh

OUT = ../out
SRC = up5k_riscv.v system.v spram_16kx32.v \
	acia.v acia_rx.v acia_tx.v \
	wb_bus.v wb_master.v \
	../picorv32/picorv32.v 

# firmware
FW_PLCHLD_HEX = fake_firmware.hex
FW_HEX = firmware.hex

# soc
PROJ = up5k_riscv
PIN_DEF = $(PROJ).pcf
SDC = $(PROJ).sdc
DEVICE = up5k
PACKAGE = sg48

YOSYS = yosys
NEXTPNR = nextpnr-ice40
ICEPACK = icepack
ICETIME = icetime
ICEPROG = iceprog
ICEBRAM = icebram
CDCPROG = cdc_prog
VERILATOR = verilator
TECH_LIB = cells_sim.v

all: $(PROJ).bin

$(FW_PLCHLD_HEX):
	@mkdir -p $(OUT);
	$(ICEBRAM) -g 32 2048 > $(OUT)/$(FW_PLCHLD_HEX)

%.json: $(SRC) $(FW_PLCHLD_HEX)
	$(YOSYS) -p 'synth_ice40 -dsp -top $(PROJ) -json $(OUT)/$@' $(SRC)

%.asc: %.json $(PIN_DEF)
	@mkdir -p $(OUT);
	$(NEXTPNR) --pre-pack $(SDC) --placer heap --timing-allow-fail --$(DEVICE) --package $(PACKAGE) --json $(OUT)/$< --pcf $(PIN_DEF) --asc $(OUT)/$@

$(FW_HEX):
	$(MAKE) -C ../firmware/ $@

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

lint: $(SRC)
	$(VERILATOR) --lint-only -Wall --top-module $(PROJ) $(TECH_LIB) $(SRC)

clean:
	$(MAKE) -C ../firmware/ clean
	rm -f $(OUT)/*.json $(OUT)/*.asc $(OUT)/*.rpt $(OUT)/*.bin

.PHONY: all prog flash recode lint clean
