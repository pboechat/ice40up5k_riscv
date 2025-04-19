MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
SCRIPTS_DIR := $(abspath $(MAKEFILE_DIR)../scripts)/

FIRMWARE := mlp
LDSCRIPT := $(MAKEFILE_DIR)mlp.ld
SOURCES := $(MAKEFILE_DIR)mlp.c

DIGITS := $(abspath $(MAKEFILE_DIR)../out/zero.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/one.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/two.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/three.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/four.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/five.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/six.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/seven.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/eight.jpg.input.bin) \
		$(abspath $(MAKEFILE_DIR)../out/nine.jpg.input.bin)

EXTRA_DEPS := $(abspath $(MAKEFILE_DIR)../out/mlp.params.bin) $(DIGITS)
			  

include $(SCRIPTS_DIR)asserts.mk
include $(SCRIPTS_DIR)firmware.mk

PARAMS_FLASH_ADDR := 204800
INPUT_FLASH_ADDR := 409600

CFLAGS += -DPARAMS_FLASH_ADDR=$(PARAMS_FLASH_ADDR) \
		  -DINPUT_FLASH_ADDR=$(INPUT_FLASH_ADDR)

CHUNK_SIZE := 204800
BITSTREAM_SIZE = $(shell stat -c %s $(OUT)mlp_up5k_soc.bin)
PARAMS_SIZE = $(shell stat -c %s $(OUT)mlp.params.bin)
PAD0_SIZE = $(shell bash -c 'echo $$(( 204800 - $(BITSTREAM_SIZE) ))')
PAD1_SIZE = $(shell bash -c 'echo $$(( 204800 - $(PARAMS_SIZE) ))')

repack: $(OUT)mlp_up5k_soc.bin $(EXTRA_DEPS)
	$(call assert-lt,BITSTREAM_SIZE,CHUNK_SIZE,"bitstream too big")
	$(call assert-lt,PARAMS_SIZE,CHUNK_SIZE,"params too big")
	@truncate -s $(PAD0_SIZE) $(OUT)pad0.bin
	@truncate -s $(PAD1_SIZE) $(OUT)pad1.bin
	@cat $(OUT)mlp_up5k_soc.bin \
		 $(OUT)pad0.bin \
		 $(OUT)mlp.params.bin \
		 $(OUT)pad1.bin \
		 $(DIGITS) > $(OUT)final_mlp_up5k_soc.bin

$(OUT)mlp.tflite:
	@mkdir -p $(OUT);
	$(SCRIPTS_DIR)python.sh $(SCRIPTS_DIR)mlp/generate_mlp_tflite.py -o $(OUT)mlp.tflite || exit 1

$(OUT)mlp.params.bin: $(OUT)mlp.tflite
	@mkdir -p $(OUT);
	$(SCRIPTS_DIR)python.sh $(SCRIPTS_DIR)mlp/extract_mlp_params.py -v -i $(OUT)mlp.tflite -o $(OUT)mlp.params.bin || exit 1

%.input.bin:
	@mkdir -p $(OUT);
	$(SCRIPTS_DIR)python.sh $(SCRIPTS_DIR)mlp/make_mlp_input.py -i $(MAKEFILE_DIR)../data/mlp/$(notdir $*) -o $@ || exit 1

$(OUT)mlp: $(HEADERS) $(SOURCES) $(OUT)mlp.params.bin $(EXTRA_DEPS)
	@mkdir -p $(OUT);
	gcc -g -o $(OUT)mlp $(SOURCES) || exit 1

.PHONY: repack