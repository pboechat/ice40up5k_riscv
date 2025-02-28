# RISC-V on the iCE40UP5K-B-EVN

RISC-V on the iCE40UP5K-B-EVN board.

Based on Eric Brombaugh's [UP5K RISC-V project](https://github.com/emeb/up5k_riscv).

## What is it?

A small RISC-V system designed to run on the iCE40UP5K-B-EVN board. 

It features:

* A [PicoRV32 CPU](https://github.com/YosysHQ/picorv32)
* 8KB of boot ROM in dedicated BRAM
* 64KB instruction/data RAM in SPRAM
* Dedicated hard IP core SPI interface to configuration flash
* Additional hard IP core SPI, currently used for an ILI9341 LCD
* Dedicated hard IP core I2C for testing
* 115.2k serial port
* 32-bit output port (for LEDs, LCD control, etc)
* GCC firmware build

## Pre-requisites

To build this you will need:

* [Yosys](https://github.com/YosysHQ/yosys) - Synthesis
* [Nextpnr](https://github.com/YosysHQ/nextpnr) - Place and Route
* [RISC-V GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) - Firmware

## Building

	git clone https://github.com/pboechat/ice40up5k_riscv.git
	cd ice40up5k_riscv
	git submodule update --init --recursive
	make

## Booting up

Connect a 115.2kbps terminal (eg, PuTTY) to the TX/RX pins of the board and apply power.

You should see a message:

	```
	up5k_riscv - starting up
	spi id: 0x00EF4016
	LCD initialized
	I2C0 Initialized
	xxxx...
	```
	
If you have an LCD connected to the SPI1 port pins on the FPGA then it should
display several different screens to demonstrate the graphics. You can store
a raw rgb565-encoded image in 240x320 dimensions at flash location 0x200000
which will be BLITed to the screen. A helper script to properly format the
image is located in the "tools" directory.

A new addition is testing of the SB_I2C hard core. If you have an I2C device
on the bus at the expected address then you will see "." characters, otherwise
"x" will be printed.

## Thanks

Thanks to:

* Eric Brombaugh <ebrombaugh1@cox.net> for the original project!
* Claire Wolf <clifford@clifford.at> for picorv32, icestorm, etc.
