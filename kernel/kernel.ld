OUTPUT_ARCH("riscv")
ENTRY(_entry)

SECTIONS
{
  . = 0x80000000;

  .text : {
    *(.text .text.*)
    PROVIDE(etext = .);
  }

  .rodata : {
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
  }

  .data : {
    *(.data .data.*)
    *(.sdata .sdata.*)
  }

  .bss : {
    . = ALIGN(16);
    _bss_start = .;
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    . = ALIGN(16);
    _bss_end = .;
  }

  PROVIDE(end = .);

  .stack : {
    . = ALIGN(16);
    stack0 = .;
    . += 4096;
  }
}