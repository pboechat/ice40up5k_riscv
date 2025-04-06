MEMORY
{
    /* 8KB ROM */
    ROM (rx)    : ORIGIN = 0x00000000, LENGTH = 0x2000
    /* 128KB RAM */
    RAM (xrw)   : ORIGIN = 0x10000000, LENGTH = 0x20000
}
SECTIONS {
    /* text section (ROM) */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.srodata)
        *(.srodata*)
        . = ALIGN(4);
        _etext = .;
        _sidata = _etext;
    } >ROM
    /* data section (RAM) */
    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .;
        _ram_start = .;
        . = ALIGN(4);
        *(.data)
        *(.data*)
        *(.sdata)
        *(.sdata*)
        . = ALIGN(4);
        _edata = .;
    } >RAM
    /* uninitialized data section (RAM) */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(.sbss)
        *(.sbss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM
    /* assert that .data + .bss <= 104KB */
    ASSERT(_ebss - _sdata <= 0x1A000, "ERROR: .data + .bss exceed 104KB!")
    /* heap section (start of RAM - after all data - grows upwards and is limited to 104KB) */
    .heap :
    {
        . = ALIGN(4);
        __heap_start = .;
        . += 0x3000; /* 12KB heap */
        __heap_end = .;
    } >RAM
    /* stack section (end of RAM - grows downwards) */
    .stack (NOLOAD) :
    {
        . = ALIGN(4);
        __stack_top = ORIGIN(RAM) + LENGTH(RAM);
        __stack_bottom = __stack_top - 0x3000;  /* 12KB stack */
    } >RAM
}
