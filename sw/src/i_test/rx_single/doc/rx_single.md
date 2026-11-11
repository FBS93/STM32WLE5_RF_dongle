# SW_INTEGRATION_TEST_1

## rx_single overview

Validate the complete single-packet receive path from host command to host report and RF idle.

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
- [Representative RX dynamic view](../../../../arch/sw_host_command_manager_sequence_diagram.md#diagram)

Status: approved

## Use case

With idle confirmed and a complete LoRa single-RX configuration applied, the host starts reception. One received packet is reported and the operation ends, as required by the [single-RX contract](../../../../../sys/req/sys_interfaces.md#rx-operation).

## Verification scope

Evidence covers configuration event fields, preservation of packet length/bytes/metadata through the event and framed host report, and RX/idle diagnostics with zero errors. Real host/RF active objects and EDF run with HAL doubles: this validates software integration, not physical reception or radio-driver correctness. Only the latest host report is captured; additional reports are not exhaustively detected. Host target: `i_test_rx_single`.
