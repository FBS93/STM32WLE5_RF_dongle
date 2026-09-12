# SW_DETAILED_DESIGN_7

## error_handler overview

`error_handler` is the project-specific fatal-error software unit. It provides the direct strong override of [`EAF_onError()`](../../../ecf/embedded_assert_framework/inc/eaf.h), which EAF invokes in critical-section context. Both variants, target and host, shall not return. The target variant asserts the active-low PA7 user LED according to [HW_SW_INTERFACE_6](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_6).

All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the Embedded Assert Framework (EAF) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.

Upstream traceability:
- [SW_ARCH_COMPONENT_2](../../../arch/sw_components.md#sw_arch_component_2)
- [SW_ARCH_COMPONENT_3](../../../arch/sw_components.md#sw_arch_component_3)
- [SW_ARCH_COMPONENT_5](../../../arch/sw_components.md#sw_arch_component_5)
- [SW_ARCH_DESIGN_1](../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2)

Hardware resources:
- PA7/user LED: Owned and accessed directly as the active-low fatal-error indication defined by [HW_SW_INTERFACE_6](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_6).

External dependencies:
- [Embedded Assert Framework (EAF)](../../../ecf/embedded_assert_framework/doc/eaf.md): Provides assertions to enforce DbC in all undefined fault handling cases and the fatal-error override contract.
- [Embedded Middleware Framework (EMF)](../../../ecf/embedded_middleware_framework/doc/emf.md): Provides utility macros for explicit unused-parameter handling.

Status: approved

## Glossary

| Term | Definition |
|---|---|
| Fatal error | Unrecoverable software error reported through EAF. |

## Usage example

Link this unit so EAF invokes its strong `EAF_onError()` override on a fatal error.
