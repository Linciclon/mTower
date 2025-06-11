MEMORY {
    ROM (rx) : ORIGIN = 0x20000, LENGTH = 0x18000
    RAM (rwx) : ORIGIN = 0x20010000, LENGTH = 0x7000
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
 . = ALIGN(32);
 _data_lma_start = .;
    . = 0x20010000;
    _data_vma_start = .;
    .data : AT(_data_lma_start){
        *(.data .data.*)
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata .sdata.* .sdata2.*)
    }
    _data_lma_end = _data_lma_start + SIZEOF(.data);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss* .sbss*)
        *(COMMON)
        __bss_end = .;
    }
    . = ALIGN(16);
    PROVIDE(_stack_base = .);
    . = . + 0x4000;
    PROVIDE(_stack_top = .);
    PROVIDE(_heap_base = .);
}
