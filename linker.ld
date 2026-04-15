SECTIONS
{
    . = 0xFFFF000000080000;
    __kernel_start = .;
    .text.boot : { *(.text.boot) }
    .text : { *(.text) }
    __kernel_code = .;
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    bss_begin = .;
    .bss : { *(.bss*) }
    bss_end = .;
    .mmmap : { *(.mmmap) }
    __kernel_end = .;
}