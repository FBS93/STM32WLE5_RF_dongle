/*******************************************************************************
 * @brief Linker file for STM32F103C8Tx series
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * STM32CubeMX (c) STMicroelectronics.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/* Entry point. */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack. */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* End of RAM. */

/* Generate a link error if heap and stack don't fit into RAM. */
_min_heap_size = 0x0; /* Required amount of heap. */
_min_stack_size = 0x400; /* Required amount of stack. */

/* Specify the memory areas. */
MEMORY
{
RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
}

/* Define output sections. */
SECTIONS
{
  /* Vector table shall be placed at the beginning of FLASH. */
  . = ALIGN(4);
  .vector_table :
  {
    KEEP(*(.vector_table)) /* Vector table. */
  } >FLASH

  /* Program code and compiler-generated data stored in FLASH. */
  . = ALIGN(4);
  .text :
  {
    *(.text) /* .text section (code). */
    *(.text*) /* .text* sections (code). */
    *(.glue_7) /* Glue arm to thumb code. */
    *(.glue_7t) /* Glue thumb to arm code. */
    *(.eh_frame) /* Exception/unwind frame info (usually unused in bare-metal C). */
    KEEP(*(.init)) /* GCC pre-main hook (usually unused in bare-metal C). */
    KEEP(*(.fini)) /* GCC post-main hook (usually unused in bare-metal C). */
    _etext = .; /* End address symbol of .text. */
  } >FLASH

  /* Constant data stored in FLASH. */
  . = ALIGN(4);
  .rodata :
  {
    *(.rodata) /* .rodata section. */
    *(.rodata*) /* .rodata* sections. */
  } >FLASH

  /* ARM exception handling tables (usually unused in bare-metal C). */
  . = ALIGN(4);
  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*) /* Unwind description tables. */
  } >FLASH

  /* ARM exception index handling tables (usually unused in bare-metal C). */
  . = ALIGN(4);
  .ARM :
  {
    __exidx_start = .; /* Start address symbol of exception index table. */
    *(.ARM.exidx*) /* Unwind index entries. */
    __exidx_end = .; /* End address symbol of exception index table. */
  } >FLASH

  /* Pre-init function pointers array (usually unused in bare-metal C). */
  . = ALIGN(4);
  .preinit_array (READONLY) :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >FLASH

  /* Init function pointers array (usually unused in bare-metal C). */
  . = ALIGN(4);
  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >FLASH

  /* Fini function pointers array (usually unused in bare-metal C). */
  . = ALIGN(4);
  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >FLASH

  /**
   * Start address in FLASH of the initial values for .data 
   * (used by startup code to initialize variables in RAM). 
   */
  _sidata = LOADADDR(.data);

  /* Initialized global/static variables placed in RAM (values loaded from FLASH). */
  . = ALIGN(4);
  .data :
  {
    _sdata = .; /* Start address symbol of .data. */
    *(.data) /* .data section. */
    *(.data*) /* .data* sections. */
    _edata = .; /* End address symbol of .data. */
  } >RAM AT> FLASH
  
  /* Uninitialized global/static variables placed in RAM (initialized to 0 by startup). */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .; /* Start address symbol of .bss. */
    __bss_start__ = _sbss; /* Alias for compatibility. */
    *(.bss) /* .bss section. */
    *(.bss*) /* .bss* sections. */
    *(COMMON) /* Uninitialized global symbols emitted as COMMON by GCC. */
    _ebss = .; /* End address symbol of .bss. */
    __bss_end__ = _ebss; /* Alias for compatibility. */
  } >RAM

  /* Heap/stack reservation used only for RAM overflow check. */
  . = ALIGN(8);
  ._user_heap_stack :
  {
    PROVIDE(end = .); /* Heap start symbol. */
    PROVIDE(_end = .); /* Alias for compatibility. */
    . = . + _min_heap_size; /* Reserve minimum heap size. */
    . = . + _min_stack_size; /* Reserve minimum stack size. */
  } >RAM
}


