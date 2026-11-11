# SW_INTEGRATION_TEST_3

## rx_continuous overview

Validate continuous RX diagnostics under queries and packet delivery, followed by replacement and explicit stop.

Upstream traceability:
- [SW_ARCH_COMPONENT_2 — swc_hostCommandManager](../../../../arch/sw_components.md#sw_arch_component_2)
- [SW_ARCH_COMPONENT_3 — swc_rfManager](../../../../arch/sw_components.md#sw_arch_component_3)
- [SW_ARCH_COMPONENT_5 — swc_edf](../../../../arch/sw_components.md#sw_arch_component_5)
- [SW_ARCH_INTERFACE_1 — swi_hostUsbControl](../../../../arch/sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_2 — swi_rfInputOutput](../../../../arch/sw_interfaces.md#sw_arch_interface_2)
- [SW_ARCH_INTERFACE_4 — swi_hostCommandManagerEdf](../../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5 — swi_rfManagerEdf](../../../../arch/sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_DESIGN_1 — swd_eventDrivenModel](../../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_PARAMETER_1 — swp_rfSemanticParameters](../../../../arch/sw_parameters.md#sw_arch_parameter_1)

Status: approved

## Use case

From confirmed idle and a complete continuous-RX configuration, reception persists through configuration/diagnostic queries and packet delivery. A packet-TX command replaces RX directly; STOP terminates that replacement, following the [command-processing contract](../../../../../sys/req/sys_interfaces.md#command-processing).

## Verification scope

Evidence includes unchanged configuration readback, RX packet bytes/metadata in event and host report, TX event/captured bytes, and distinct RX/TX/idle diagnostics preserving supplied `0x0140` error flags. Real host/RF active objects and EDF run with HAL doubles; operation identity does not prove uninterrupted physical RX or the HAL stop/start ordering during replacement. Extra events/reports are not exhaustively detected. Host target: `i_test_rx_continuous`.
