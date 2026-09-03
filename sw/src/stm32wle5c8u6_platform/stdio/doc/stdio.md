# Stdio overview

`stdio` provides non-blocking EBF stdout for the STM32WLE5C8U6 target through ITM, TPIU and SWO. Target stdin is not supported.

Initialization shall enable the MCU trace path and the ITM stimulus output used by stdout, preserve PB3 for SWO, select the TPIU NRZ asynchronous protocol and derive the TPIU prescaler from the active trace clock for the nominal 2 Mbit/s SWO bitrate. Clock and pin constraints remain defined by [SW_ARCH_DESIGN_2](../../../../arch/sw_designs.md#sw_arch_design_2), the [HW/SW interfaces](../../../../../hw/cross_domain_interfaces/hw_sw_interface.md) and the referenced MCU documentation.

Producers enqueue bytes in a statically allocated 1024-byte TX FIFO. Admission is non-blocking and all-or-nothing: `EBF_stdoutWrite()` enqueues the complete message when capacity is available or returns `false` without waiting for hardware.

TIM2 is dedicated to consuming the FIFO from a low-priority ISR, below the radio and USART2 interrupt priorities. The timer has a 1 ms period, starts when the FIFO changes from empty to non-empty, and stops when the FIFO becomes empty. Each ISR transmits as many queued bytes as possible in FIFO order without waiting for hardware or blocking. A byte is removed only when the selected ITM stimulus port `FIFOREADY` condition is asserted; otherwise, the ISR exits immediately and retries on the next period.

The implementation shall comply with the [EBF stdin/stdout critical-section contract](../../../../ecf/embedded_base_framework/doc/ebf.md).

All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the Embedded Assert Framework (EAF) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.

# Glossary

| Term | Definition |
|---|---|
| ITM | Instrumentation Trace Macrocell. |
| TPIU | Trace Port Interface Unit. |
| SWO | Serial Wire Output. |
| NRZ | Non-return-to-zero asynchronous trace encoding. |

# Usage example

See the [EBF usage example](../../../../ecf/embedded_base_framework/doc/ebf.md#usage-example).
