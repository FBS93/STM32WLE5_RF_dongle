# SW_INTEGRATION_TEST_4

## tx_completion overview

Validate asynchronous packet-TX completion and return to idle through real EDF event delivery.

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

From confirmed idle and a complete LoRa configuration, the host requests one packet transmission. The operation remains active until a simulated radio completion indication ends it, without a host completion message, consistent with the [host protocol](../../../../../sys/req/sys_interfaces.md#tx-packet-request).

## Verification scope

Evidence covers configuration event fields, packet length/bytes through the TX event and HAL boundary, OPERATION_COMPLETE delivery, and packet-TX/idle diagnostic events and reports with zero errors. Real host/RF active objects and EDF run with HAL doubles: captured transmit bytes do not demonstrate physical transmission. Extra events/reports are not exhaustively detected. Host target: `i_test_tx_completion`.
