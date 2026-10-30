MEMORY {
    RAM (rwx) : ORIGIN = 0x10100000, LENGTH = 0x30000
}
ENTRY(_start)
SECTIONS {
    .start : {
        *(.start)
    }
    .text : {
        *(.text*)
    }
    .rodata : ALIGN(4) {
  __rodata_start = .;
  *(.rodata .rodata.*)
  . = ALIGN(4);
  __start_ta_head_section = . ;
  KEEP(*(ta_head_section))
  __stop_ta_head_section = . ;
  . = ALIGN(4);
  __start_user_ta_head_section = . ;
  KEEP(*(user_ta_head_section))
  __stop_user_ta_head_section = . ;
  . = ALIGN(4);
  __rodata_end = .;
    }
    .ARM.extab : ALIGN(4) {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    }
    .ARM.exidx : ALIGN(4) {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    }
    .eh_frame : ALIGN(4) {
        KEEP(*(.eh_frame*))
    }
 . = ALIGN(32);
    .data : {
        _data_vma_start = .;
        _data_lma_start = .;
        *(.data .data.*)
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata .sdata.* .sdata2.*)
        _data_vma_end = .;
    }
    _data_lma_end = _data_vma_end;
    ASSERT(LOADADDR(.data) == ADDR(.data),
           "unified-memory .data has different VMA and LMA")
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss* .sbss*)
        *(COMMON)
        __bss_end = .;
    }
 .stack (NOLOAD) : {
  . = ALIGN(16);
  PROVIDE(_stack_base = .);
  . = . + (2 * 0x4000 * 1);
  PROVIDE(_stack_top = .);
 } > RAM
 PROVIDE(_heap_base = .);
    ASSERT(_stack_top <= ORIGIN(RAM) + LENGTH(RAM),
           "RAM overflow: stack exceeds PLAT_MEM_SIZE")
}
