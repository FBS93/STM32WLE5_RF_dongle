# SW_DETAILED_DESIGN_5

## stm32wle5c8u6_platform overview

`stm32wle5c8u6_platform` is the project-specific software platform unit for the STM32WLE5C8U6 target. Its scope is the target support defined by [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2), subject to the canonical hardware constraints in the [HW/SW interfaces](../../../../hw/cross_domain_interfaces/hw_sw_interface.md). The [stdio](../stdio/doc/stdio.md) subunit defines target stdout integration.

Upstream traceability:
- [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2)

Hardware resources:
- STM32WLE5C8U6 MCU: Target resources and connections are defined by the [HW/SW interfaces](../../../../hw/cross_domain_interfaces/hw_sw_interface.md).

External dependencies:
- None.

Status: not approved

## Glossary

| Term | Definition |
|---|---|
| | |

## Usage example

Use this unit for STM32WLE5C8U6-specific target support and its [stdio](../stdio/doc/stdio.md) subunit for stdout target integration.
