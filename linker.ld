SECTIONS
{
    .text.boot : { *(.text.boot) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    bss_begin = .;
    .bss : { *(.bss*) }
    bss_end = .;
    . = ALIGN(0x00001000);
    pgd = .;
    .data.pgd : { . += (6 * (1 << 12)); }
}