/*******************************************************************************
 * @brief Linker script for STM32WLE5C8U6.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file follows the ECF STM32F103C8Tx linker-script pattern.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
  RAM (xrw) : ORIGIN = 0x20008000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_min_heap_size = 0x0;
_min_stack_size = 0x400;

SECTIONS
{
  . = ALIGN(4);
  .vector_table :
  {
    KEEP(*(.vector_table))
  } >FLASH

  . = ALIGN(4);
  .text :
  {
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    _etext = .;
  } >FLASH

  . = ALIGN(4);
  .rodata :
  {
    *(.rodata)
    *(.rodata*)
  } >FLASH

  . = ALIGN(4);
  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH

  . = ALIGN(4);
  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  . = ALIGN(4);
  .preinit_array (READONLY) :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >FLASH

  . = ALIGN(4);
  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >FLASH

  . = ALIGN(4);
  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >FLASH

  _sidata = LOADADDR(.data);

  . = ALIGN(4);
  .data :
  {
    _sdata = .;
    *(.data)
    *(.data*)
    _edata = .;
  } >RAM AT> FLASH

  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  . = ALIGN(8);
  ._user_heap_stack :
  {
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + _min_heap_size;
    . = . + _min_stack_size;
  } >RAM
}
