OUTPUT_ARCH("riscv")
ENTRY(_entry)

SECTIONS
{
  . = 0x80000000;               /* QEMU kernel load address */

  .text : {
    *(.text .text.*)            /* All text sections */
    PROVIDE(etext = .);         /* End of text */
  }

  .rodata : {
    *(.rodata .rodata.*)        /* Read-only data */
    *(.srodata .srodata.*)
  }

  .data : {
    *(.data .data.*)            /* Initialized data */
    *(.sdata .sdata.*)
  }

  .bss : {
    . = ALIGN(16);
    _bss_start = .;             /* Start of BSS */
    *(.bss .bss.*)              /* Uninitialized data */
    *(.sbss .sbss.*)
    . = ALIGN(16);
    _bss_end = .;               /* End of BSS */
  }

  PROVIDE(end = .);             /* End of kernel */

  /* Stack space */
  .stack : {
    . = ALIGN(16);
    stack0 = .;
    . += 4096;                  /* 4KB stack */
  }
}