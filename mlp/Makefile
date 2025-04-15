MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
SCRIPTS_DIR := $(abspath $(MAKEFILE_DIR)../scripts)/

FIRMWARE := mlp
LDSCRIPT := $(MAKEFILE_DIR)mlp.ld
SOURCES := $(MAKEFILE_DIR)mlp.c

EXTRA_DEPS := $(abspath $(MAKEFILE_DIR)../out/mlp.params.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/zero.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/one.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/two.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/three.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/four.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/five.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/six.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/seven.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/eight.jpg.input.bin) \
			  $(abspath $(MAKEFILE_DIR)../out/nine.jpg.input.bin)

include $(SCRIPTS_DIR)firmware.mk

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
