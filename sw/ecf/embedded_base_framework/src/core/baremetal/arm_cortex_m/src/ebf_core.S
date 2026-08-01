/*******************************************************************************
 * @brief Core/port-specific base implementation.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
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

.extern EAF_onError

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * DATA SECTION
 ******************************************************************************/

.section .data
.align 2
/**
 * Interrupts may be disabled by code outside the EBF framework. Therefore, EBF
 * maintains its own internal flag to track whether a critical section has been
 * explicitly entered via EBF_entryCriticalSection(). This ensures that
 * EBF_isInCriticalSection() reflects only the framework-managed critical state,
 * not the hardware PRIMASK state.
 */
.LisInCriticalSection: .byte 0

.section .rodata
.align 2
.Lfile: .asciz __FILE__

/*******************************************************************************
 * CODE SECTION
 ******************************************************************************/

.section .text
.align 4

#if (__ARM_ARCH > 6)
.global EBF_utils_log2
.type EBF_utils_log2, %function
EBF_utils_log2:
    AND     r0, r0, #0xFF // Ensure only low byte is used.
    CLZ     r1, r0        // r1 = count leading zeros of r0.
    MOV     r2, #32
    SUB     r0, r2, r1 // r0 = 32 - CLZ(r0)
    BX      lr
.size EBF_utils_log2, .-EBF_utils_log2
#endif /* (__ARM_ARCH > 6) */

.global EBF_entryCriticalSection
.type EBF_entryCriticalSection, %function
EBF_entryCriticalSection:
    MRS     r0, PRIMASK // r0 = current PRIMASK value.
    CPSID   i           // Disable interrupts globally.
    CMP     r0, #0      // Were interrupts already disabled?
    BNE     EBF_entryCriticalSectionError // If yes, go to error handler.
    LDR     r1, =.LisInCriticalSection
    MOVS    r2, #1
    STRB    r2, [r1] // .LisInCriticalSection = true
    BX      lr
EBF_entryCriticalSectionError:
    LDR     r0, =.Lfile
    LDR     r1, =__LINE__
    BL      EAF_onError
.size EBF_entryCriticalSection, .-EBF_entryCriticalSection

.global EBF_exitCriticalSection
.type EBF_exitCriticalSection, %function
EBF_exitCriticalSection:
    MRS     r0, PRIMASK // r0 = current PRIMASK value.
    CMP     r0, #0      // Were interrupts disabled?
    BEQ     EBF_exitCriticalSectionError // If not, go to error handler.
    LDR     r1, =.LisInCriticalSection
    MOVS    r2, #0
    STRB    r2, [r1]    // .LisInCriticalSection = false
    CPSIE   i // Enable interrupts globally.
    ISB       // Synchronize after enabling IRQs.
    BX      lr
EBF_exitCriticalSectionError:
    LDR     r0, =.Lfile
    LDR     r1, =__LINE__
    BL      EAF_onError
.size EBF_exitCriticalSection, .-EBF_exitCriticalSection

.global EBF_isInCriticalSection
.type EBF_isInCriticalSection, %function
EBF_isInCriticalSection:
    LDR     r0, =.LisInCriticalSection
    LDRB    r0, [r0]
    BX      lr
.size EBF_isInCriticalSection, .-EBF_isInCriticalSection

.global EBF_noOperation
.type EBF_noOperation, %function
EBF_noOperation:
    NOP
    BX lr
.size EBF_noOperation, .-EBF_noOperation