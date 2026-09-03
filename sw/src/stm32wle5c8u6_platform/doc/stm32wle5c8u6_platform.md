# SW_DETAILED_DESIGN_5

## stm32wle5c8u6_platform overview

`stm32wle5c8u6_platform` is the project-specific software platform unit for the STM32WLE5C8U6 target. Its scope is the target support defined by [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2), subject to the canonical hardware constraints in the [HW/SW interfaces](../../../../hw/cross_domain_interfaces/hw_sw_interface.md). The [stdio](../stdio/doc/stdio.md) subunit defines target stdout integration.

Upstream traceability:
- [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2)

Hardware resources:
- STM32WLE5C8U6 MCU: Target resources and connections are defined by the [HW/SW interfaces](../../../../hw/cross_domain_interfaces/hw_sw_interface.md).
- ITM, TPIU, and PB3/SWO: Provide the target stdout path; the board connection is defined by [HW_SW_INTERFACE_4](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_4), and MCU operation is defined by [EHW_DOC_1](../../../../spec/external_hw_docs.md#ehw_doc_1).
- TIM2: Provides the periodic interrupt used to drain the TX FIFO.

External dependencies:
- [Embedded Base Framework (EBF)](../../../ecf/embedded_base_framework/doc/ebf.md): Provides the stdout override API, weak stdin behavior, and critical-section contract.
- [Embedded Assert Framework (EAF)](../../../ecf/embedded_assert_framework/doc/eaf.md): Provides assertions to enforce DbC in all undefined fault handling cases.

Status: approved

## Glossary

| Term | Definition |
|---|---|
| | |

## Usage example

Use this unit for STM32WLE5C8U6-specific target support and its [stdio](../stdio/doc/stdio.md) subunit for stdout target integration.
