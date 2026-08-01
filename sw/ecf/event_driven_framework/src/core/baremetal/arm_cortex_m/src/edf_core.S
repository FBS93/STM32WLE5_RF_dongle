/*******************************************************************************
 * @brief Event Driven Framework (EDF) core/port-specific implementation.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * QP/C (c) Quantum Leaps, LLC.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * ASSEMBLER SETTINGS
 ******************************************************************************/

.syntax unified
.thumb

/*******************************************************************************
 * EXTERNAL SYMBOLS
 ******************************************************************************/

.extern EBF_entryCriticalSection
.extern EBF_exitCriticalSection
.extern EDF_core_activate
.extern EAF_onError

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

.equ SCB_ICSR, 0xE000ED04 // Interrupt Control and State Register

/*******************************************************************************
 * DATA SECTION
 ******************************************************************************/

.section .rodata
.align 2

.Lfile: .asciz __FILE__

/*******************************************************************************
 * CODE SECTION
 ******************************************************************************/

.section .text
.align 4

.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
    // Save stack aligner (r0) and EXC_RETURN (lr).
    PUSH    {r0, lr}

    // Disable interrupts (enter critical section).
    LDR     r0, =EBF_entryCriticalSection
    BLX     r0

    // Unpend any spurious PendSV.
    LDR     r2, =SCB_ICSR
    MOV     r1, #1
    LSL     r1, r1, #27 // r1 := (1 << 27)
    STR     r1, [r2]    // Clear PendSV pending bit.

    // Prepare return from exception into Thread mode -> EDF_core_activate().
    LSR     r3, r1, #3             // r3 := (r1 >> 3), sets T (Thumb) bit in xPSR.
    LDR     r2, =EDF_core_activate // Address of EDF_core_activate.
    SUB     r2, r2, #1             // Align Thumb address of EDF_core_activate (clear bit0).
    LDR     r1, =EDF_core_preemptedContextReturn // Return address after EDF_core_activate.

    // Reserve 8 words for fabricated exception frame.
    SUB    sp, sp, #(8*4)

    // Write xPSR, PC, LR to fabricated exception frame.
    ADD     r0, sp, #(5*4)
    STM     r0!, {r1-r3}

    // Fabricate EXC_RETURN for Thread mode using MSP.
    MOV     r0, #6
    MVN    r0, r0 // r0 := 0xFFFFFFF9

#if (__ARM_ARCH != 6)
    DSB        // ARM erratum 838869.
#endif
    BX      r0 // Exception-return to EDF_core_activate().
.size PendSV_Handler, .-PendSV_Handler


.global EDF_core_preemptedContextReturn
.type EDF_core_preemptedContextReturn, %function
EDF_core_preemptedContextReturn:
#if (__ARM_ARCH > 6)
#ifdef __ARM_FP
    // --------- ARMv7-M and higher with FPU ---------
    // Clear CONTROL.FPCA bit to avoid using FP stack frame.
    MRS     r0, CONTROL
    BIC     r0, r0, #4
    MSR     CONTROL, r0
    ISB // Synchronize after CONTROL write.
#endif
#endif
    // Prepare NMI Trigger.
    LDR     r0, =SCB_ICSR // Interrupt Control and State Register.
    MOV     r1, #1
    LSL     r1, r1, #31 // r1 := (1 << 31) => Pend NMI.
    STR     r1, [r0]    // Trigger NMI.

    // Execution continues in NMI_Handler (interrupts remain disabled).

    // If execution reaches here, raise error.
    LDR     r0, =.Lfile
    LDR     r1, =__LINE__
    BL      EAF_onError
.size EDF_core_preemptedContextReturn, .-EDF_core_preemptedContextReturn


.global NMI_Handler
.type NMI_Handler, %function
NMI_Handler:
    // Remove 8 words of fabricated exception frame.
    ADD    sp, sp, #(8*4)

    // Enable interrupts (exit critical section).
    BL      EBF_exitCriticalSection

    /**
     * lr (EXC_RETURN) was clobbered. Pop stack aligner + EXC_RETURN
     * into PC (Pushed in PendSV_Handler).
     */
    POP     {r0, pc}
.size NMI_Handler, .-NMI_Handler


.global EDF_core_disableFPUContext
.type EDF_core_disableFPUContext, %function
EDF_core_disableFPUContext:
    MRS     r0, CONTROL // r0 = CONTROL register.
    BIC     r0, r0, #4  // clear FPCA bit (bit 2).
    MSR     CONTROL, r0 // write back CONTROL.
    ISB                 // Synchronize after CONTROL write.
    BX      lr
.size EDF_core_disableFPUContext, .-EDF_core_disableFPUContext


.global EDF_core_getIPSR
.type EDF_core_getIPSR, %function
EDF_core_getIPSR:
    MRS     r0, IPSR // Read IPSR into r0.
    BX      lr
.size EDF_core_getIPSR, .-EDF_core_getIPSR


.global EDF_core_ARMErratum838869
.type   EDF_core_ARMErratum838869, %function
EDF_core_ARMErratum838869:
    DSB // Data Synchronization Barrier.
    BX      lr
.size   EDF_core_ARMErratum838869, .-EDF_core_ARMErratum838869