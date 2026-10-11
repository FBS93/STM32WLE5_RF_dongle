# SW_DETAILED_DESIGN_5

## stm32wle5c8u6_platform overview

`stm32wle5c8u6_platform` is the project-specific software platform unit for the STM32WLE5C8U6 target. Its scope is the target support defined by [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2), subject to the canonical hardware constraints in the [HW/SW interfaces](../../../../hw/cross_domain_interfaces/hw_sw_interface.md). The [stdio](../stdio/doc/stdio.md) subunit defines target stdout integration.

All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the [Embedded Assert Framework (EAF)](../../../ecf/embedded_assert_framework/doc/eaf.md) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.

Upstream traceability:
- [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2)

Hardware resources:
- RAM and SWD: Hold the RTT control block and buffers and allow the debug probe to read stdout; debug-pin reservations remain defined by [HW_SW_INTERFACE_4](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_4).

External dependencies:
- [Embedded Base Framework (EBF)](../../../ecf/embedded_base_framework/doc/ebf.md): Provides the stdout override API, weak stdin behavior, and critical-section contract.
- [Embedded Assert Framework (EAF)](../../../ecf/embedded_assert_framework/doc/eaf.md): Provides assertions to enforce Design by Contract (DbC) in all undefined fault handling cases.
- [SEGGER RTT](../../../third_party/segger_rtt-8.58.0/README.md): Provides the target stdout transport configured by the [stdio](../stdio/doc/stdio.md) software unit.

Status: approved

## Glossary

| Term | Definition |
|---|---|
| | |

## Usage example

Use this unit for STM32WLE5C8U6-specific target support and its [stdio](../stdio/doc/stdio.md) subunit for stdout target integration.
