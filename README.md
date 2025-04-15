# RISC-V SoC on the iCE40UP5K-B-EVN

A RISC-V SoC design on the iCE40UP5K-B-EVN board.

Based on Eric Brombaugh's [UP5K RISC-V project](https://github.com/emeb/up5k_riscv).

## What is it?

A small RISC-V SoC designed to run on the iCE40UP5K-B-EVN board. 

It features:

* A [PicoRV32 CPU](https://github.com/YosysHQ/picorv32)
* 8KB of boot ROM in dedicated BRAM
* 128KB instruction/data RAM in SPRAM
* Dedicated hard IP core SPI interface to configuration flash
* Additional hard IP core SPI, currently used for an ILI9341 LCD
* Dedicated hard IP core I2C for testing
* 115.2k serial port
* 32-bit output port (for LEDs, LCD control, etc)
* A minimalistic firmware SDK

## Pre-requisites

To build and use this project, ensure the following dependencies are installed:

* [yosys](https://github.com/YosysHQ/yosys) - Synthesis
* [nextpnr](https://github.com/YosysHQ/nextpnr) - Place and Route
* [RISC-V GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) - Firmware
* [picolibc](https://github.com/picolibc/picolibc) - Firmware

To install these dependencies, follow the installation instructions linked above or check your system's package manager.


## Building

	git clone https://github.com/pboechat/ice40up5k_riscv.git
	cd ice40up5k_riscv
	git submodule update --init --recursive
	make

Alternatively, you can pass `FIRMWARE=<relpath>` to build the SoC with a different firmware.


## Firmwares

- [demo](demo/) - A simple firmware to demonstrate the capabilities of the RISC-V SoC.
- [mlp](mlp/) - A minimalistic multi-layer perceptron implementation running on the SoC.


## Thanks

- Eric Brombaugh for the original [UP5K RISC-V project](https://github.com/emeb/up5k_riscv).
- Claire Wolf for [picorv32](https://github.com/YosysHQ/picorv32), [icestorm](https://github.com/cliffordwolf/icestorm), and other contributions.
