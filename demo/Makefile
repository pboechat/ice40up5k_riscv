MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

FIRMWARE := demo
LDSCRIPT := $(MAKEFILE_DIR)demo.ld
SOURCES := $(MAKEFILE_DIR)demo.c

include $(MAKEFILE_DIR)../scripts/firmware.mk
