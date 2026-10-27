/** 
 * Bao, a Lightweight Static Partitioning Hypervisor 
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *      Sandro Pinto <sandro.pinto@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details. 
 *
 */

#include <core.h>
#include <plat.h>

#ifndef PLAT_CPU_COUNT
#define PLAT_CPU_COUNT 1
#endif

#ifndef STACK_SIZE
#define STACK_SIZE 0x4000
#endif

MEMORY {
#if defined(PLAT_NON_UNIFIED_MEM)
    ROM (rx)     : ORIGIN = PLAT_CODE_MEM_BASE, LENGTH = PLAT_CODE_MEM_SIZE
#endif
    RAM (rwx)    : ORIGIN = MEM_BASE, LENGTH = MEM_SIZE
}

ENTRY(_start)

SECTIONS {

    .start : {
        *(.start)
    }

    .text :  {
        *(.text*)
    }

    .rodata :  ALIGN(4) {
		__rodata_start = .;
		*(.rodata .rodata.*)
		/*
		 * 8 to avoid unwanted padding between __start_ta_head_section
		 * and the first structure in ta_head_section, in 64-bit
		 * builds
		 */
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

    /*
     * Keep the ARM unwind tables before computing the load address of .data.
     * If these sections are left orphaned, ld may insert them after the
     * _data_lma_start assignment and give .data different VMA/LMA addresses.
     */
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

	. = ALIGN(32); /* start RW sections in separate page */

#if defined(PLAT_NON_UNIFIED_MEM)
    _data_lma_start = .;
    . = PLAT_MEM_BASE;
#endif

#if defined(PLAT_NON_UNIFIED_MEM)
    .data : AT(_data_lma_start) {
#else
    .data : {
#endif
        _data_vma_start = .;
#if !defined(PLAT_NON_UNIFIED_MEM)
        _data_lma_start = .;
#endif
        *(.data .data.*)
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata .sdata.* .sdata2.*)
        _data_vma_end = .;
    }

#if defined(PLAT_NON_UNIFIED_MEM)
    _data_lma_end = _data_lma_start + SIZEOF(.data);
#else
    _data_lma_end = _data_vma_end;
    ASSERT(LOADADDR(.data) == ADDR(.data),
           "unified-memory .data has different VMA and LMA")
#endif

    .bss (NOLOAD) :  {
        __bss_start = .;
        *(.bss* .sbss*)
        *(COMMON)
        __bss_end = .;
    }

	.stack (NOLOAD) : {
		. = ALIGN(16);
		PROVIDE(_stack_base = .);
		. = . + (2 * STACK_SIZE * PLAT_CPU_COUNT);
		PROVIDE(_stack_top = .);
	} > RAM
	PROVIDE(_heap_base = .);

    ASSERT(_stack_top <= ORIGIN(RAM) + LENGTH(RAM),
           "RAM overflow: stack exceeds PLAT_MEM_SIZE")
}
