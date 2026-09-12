# SW_DETAILED_DESIGN_6

## main overview

`main` is the project-specific composition and startup software unit. It owns `main`, the static application and EDF storage, and one instance each of `hostCommandManager_t` and `rfManager_t`. It calls `EDF_init()`, initializes the platform and required EDF structures, initializes and starts both active objects while event-producing sources remain disabled, and transfers control to `EDF_run()`. Event-producing sources shall be initialized and enabled only at the end of `EDF_onStartup()`, in accordance with the [EDF startup contract](../../../ecf/event_driven_framework/doc/edf.md#startup-contract-for-event-sources). The unit is built directly as the final executable for host builds and the supported STM32WLE5 target build.

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
- None. This unit does not access hardware directly.

External dependencies:
- Embedded Assert Framework (EAF): Provides assertions to enforce Design by Contract (DbC) in all undefined fault handling cases.
- Event Driven Framework (EDF): Provides framework initialization, static EDF structures, active-object startup, the startup callback, and the run loop.
- Embedded Middleware Framework (EMF): Provides aligned storage for the mutable-event pool.
- [host_command_manager](../../host_command_manager/doc/host_command_manager.md): Provides the host-control active object.
- [rf_manager](../../rf_manager/doc/rf_manager.md): Provides the RF active object.
- [stm32wle5c8u6_platform](../../stm32wle5c8u6_platform/doc/stm32wle5c8u6_platform.md): Provides support in the target variant; host builds use the existing ECF POSIX port.

Status: approved

## Glossary

| Term | Definition |
|---|---|
| Event-producing source | An asynchronous source capable of publishing an EDF event. |

## Usage example

Use this unit as the target executable entry point and application composition root; follow the [EDF startup contract](../../../ecf/event_driven_framework/doc/edf.md#startup-contract-for-event-sources).
