/*******************************************************************************
 * @brief Startup and vector table for STM32WLE5C8U6.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file follows the ECF startup pattern and the ST STM32WLE5xx vector
 * order.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * ASSEMBLER SETTINGS
 ******************************************************************************/

.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

/*******************************************************************************
 * EXTERNAL SYMBOLS
 ******************************************************************************/

.weak SystemInit

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * DATA SECTION
 ******************************************************************************/

/*******************************************************************************
 * CODE SECTION
 ******************************************************************************/

.section .text.Reset_Handler
.global Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
  ldr sp, =_estack
  bl SystemInit

  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
copy_data_loop:
  cmp r0, r1
  bcs zero_bss_start
  ldr r3, [r2], #4
  str r3, [r0], #4
  b copy_data_loop

zero_bss_start:
  ldr r0, =_sbss
  ldr r1, =_ebss
  movs r2, #0
zero_bss_loop:
  cmp r0, r1
  bcs start_runtime
  str r2, [r0], #4
  b zero_bss_loop

start_runtime:
  bl __libc_init_array
  bl main
reset_returned:
  b reset_returned
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.global Default_Handler
.type Default_Handler, %function
Default_Handler:
default_loop:
  b default_loop
.size Default_Handler, .-Default_Handler

.macro WEAK_DEFAULT handler
  .weak \handler
  .thumb_set \handler, Default_Handler
.endm

.section .vector_table,"a",%progbits
.global Vector_Table
.type Vector_Table, %object
Vector_Table:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  .word WWDG_IRQHandler
  .word PVD_PVM_IRQHandler
  .word TAMP_STAMP_LSECSS_SSRU_IRQHandler
  .word RTC_WKUP_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI4_IRQHandler
  .word DMA1_Channel1_IRQHandler
  .word DMA1_Channel2_IRQHandler
  .word DMA1_Channel3_IRQHandler
  .word DMA1_Channel4_IRQHandler
  .word DMA1_Channel5_IRQHandler
  .word DMA1_Channel6_IRQHandler
  .word DMA1_Channel7_IRQHandler
  .word ADC_IRQHandler
  .word DAC_IRQHandler
  .word 0
  .word COMP_IRQHandler
  .word EXTI9_5_IRQHandler
  .word TIM1_BRK_IRQHandler
  .word TIM1_UP_IRQHandler
  .word TIM1_TRG_COM_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM16_IRQHandler
  .word TIM17_IRQHandler
  .word I2C1_EV_IRQHandler
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word LPUART1_IRQHandler
  .word LPTIM1_IRQHandler
  .word LPTIM2_IRQHandler
  .word EXTI15_10_IRQHandler
  .word RTC_Alarm_IRQHandler
  .word LPTIM3_IRQHandler
  .word SUBGHZSPI_IRQHandler
  .word 0
  .word 0
  .word HSEM_IRQHandler
  .word I2C3_EV_IRQHandler
  .word I2C3_ER_IRQHandler
  .word SUBGHZ_Radio_IRQHandler
  .word AES_IRQHandler
  .word RNG_IRQHandler
  .word PKA_IRQHandler
  .word DMA2_Channel1_IRQHandler
  .word DMA2_Channel2_IRQHandler
  .word DMA2_Channel3_IRQHandler
  .word DMA2_Channel4_IRQHandler
  .word DMA2_Channel5_IRQHandler
  .word DMA2_Channel6_IRQHandler
  .word DMA2_Channel7_IRQHandler
  .word DMAMUX1_OVR_IRQHandler
.size Vector_Table, .-Vector_Table

WEAK_DEFAULT NMI_Handler
WEAK_DEFAULT HardFault_Handler
WEAK_DEFAULT MemManage_Handler
WEAK_DEFAULT BusFault_Handler
WEAK_DEFAULT UsageFault_Handler
WEAK_DEFAULT SVC_Handler
WEAK_DEFAULT DebugMon_Handler
WEAK_DEFAULT PendSV_Handler
WEAK_DEFAULT SysTick_Handler
WEAK_DEFAULT WWDG_IRQHandler
WEAK_DEFAULT PVD_PVM_IRQHandler
WEAK_DEFAULT TAMP_STAMP_LSECSS_SSRU_IRQHandler
WEAK_DEFAULT RTC_WKUP_IRQHandler
WEAK_DEFAULT FLASH_IRQHandler
WEAK_DEFAULT RCC_IRQHandler
WEAK_DEFAULT EXTI0_IRQHandler
WEAK_DEFAULT EXTI1_IRQHandler
WEAK_DEFAULT EXTI2_IRQHandler
WEAK_DEFAULT EXTI3_IRQHandler
WEAK_DEFAULT EXTI4_IRQHandler
WEAK_DEFAULT DMA1_Channel1_IRQHandler
WEAK_DEFAULT DMA1_Channel2_IRQHandler
WEAK_DEFAULT DMA1_Channel3_IRQHandler
WEAK_DEFAULT DMA1_Channel4_IRQHandler
WEAK_DEFAULT DMA1_Channel5_IRQHandler
WEAK_DEFAULT DMA1_Channel6_IRQHandler
WEAK_DEFAULT DMA1_Channel7_IRQHandler
WEAK_DEFAULT ADC_IRQHandler
WEAK_DEFAULT DAC_IRQHandler
WEAK_DEFAULT COMP_IRQHandler
WEAK_DEFAULT EXTI9_5_IRQHandler
WEAK_DEFAULT TIM1_BRK_IRQHandler
WEAK_DEFAULT TIM1_UP_IRQHandler
WEAK_DEFAULT TIM1_TRG_COM_IRQHandler
WEAK_DEFAULT TIM1_CC_IRQHandler
WEAK_DEFAULT TIM2_IRQHandler
WEAK_DEFAULT TIM16_IRQHandler
WEAK_DEFAULT TIM17_IRQHandler
WEAK_DEFAULT I2C1_EV_IRQHandler
WEAK_DEFAULT I2C1_ER_IRQHandler
WEAK_DEFAULT I2C2_EV_IRQHandler
WEAK_DEFAULT I2C2_ER_IRQHandler
WEAK_DEFAULT SPI1_IRQHandler
WEAK_DEFAULT SPI2_IRQHandler
WEAK_DEFAULT USART1_IRQHandler
WEAK_DEFAULT USART2_IRQHandler
WEAK_DEFAULT LPUART1_IRQHandler
WEAK_DEFAULT LPTIM1_IRQHandler
WEAK_DEFAULT LPTIM2_IRQHandler
WEAK_DEFAULT EXTI15_10_IRQHandler
WEAK_DEFAULT RTC_Alarm_IRQHandler
WEAK_DEFAULT LPTIM3_IRQHandler
WEAK_DEFAULT SUBGHZSPI_IRQHandler
WEAK_DEFAULT HSEM_IRQHandler
WEAK_DEFAULT I2C3_EV_IRQHandler
WEAK_DEFAULT I2C3_ER_IRQHandler
WEAK_DEFAULT SUBGHZ_Radio_IRQHandler
WEAK_DEFAULT AES_IRQHandler
WEAK_DEFAULT RNG_IRQHandler
WEAK_DEFAULT PKA_IRQHandler
WEAK_DEFAULT DMA2_Channel1_IRQHandler
WEAK_DEFAULT DMA2_Channel2_IRQHandler
WEAK_DEFAULT DMA2_Channel3_IRQHandler
WEAK_DEFAULT DMA2_Channel4_IRQHandler
WEAK_DEFAULT DMA2_Channel5_IRQHandler
WEAK_DEFAULT DMA2_Channel6_IRQHandler
WEAK_DEFAULT DMA2_Channel7_IRQHandler
WEAK_DEFAULT DMAMUX1_OVR_IRQHandler
