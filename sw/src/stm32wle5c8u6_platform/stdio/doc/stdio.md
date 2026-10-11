# Stdio overview

`stdio` provides non-blocking EBF stdout for the STM32WLE5C8U6 target through [SEGGER RTT](../../../../third_party/segger_rtt-8.58.0/README.md) over SWD, as defined by [SW_ARCH_DESIGN_2](../../../../arch/sw_designs.md#sw_arch_design_2). The EBF API is unchanged; target stdin is not supported and retains the EBF weak behavior.

The first valid stdout write shall initialize RTT once using `SEGGER_RTT_Init()`. Initialization and each `SEGGER_RTT_WriteNoLock()` call shall be protected by the [EBF stdin/stdout critical-section contract](../../../../ecf/embedded_base_framework/doc/ebf.md).

Writes use RTT up-channel 0 with `SEGGER_RTT_MODE_NO_BLOCK_SKIP`: `EBF_stdoutWrite()` returns `true` only when the complete message is queued, or `false` without queuing bytes when capacity is insufficient. A null data pointer or zero length returns `false`. Admission is non-blocking and all-or-nothing; it does not wait for the debug probe.

The statically allocated up-buffer has 1024 bytes of usable capacity: [CMake configuration](../../../../third_party/CMakeLists.txt) sets `BUFFER_SIZE_UP=1025` because RTT reserves one byte. RTT buffer and write semantics remain defined by the [upstream implementation](../../../../third_party/segger_rtt-8.58.0/RTT/SEGGER_RTT.c).

The debug probe drains the buffer from SRAM over SWD.

All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the Embedded Assert Framework (EAF) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.

# Glossary

| Term | Definition |
|---|---|
| RTT | Real Time Transfer. |

# Usage example

See the [EBF usage example](../../../../ecf/embedded_base_framework/doc/ebf.md#usage-example).
