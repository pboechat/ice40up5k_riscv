MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

LDSCRIPT := $(MAKEFILE_DIR)/demo.ld
HEADERS :=
SOURCES := $(MAKEFILE_DIR)/demo.c

include ../scripts/firmware.mk
