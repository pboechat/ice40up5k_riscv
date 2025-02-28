all:
	$(MAKE) -C up5k_riscv/ all

clean:
	$(MAKE) -C up5k_riscv/ clean

.PHONY: all clean
