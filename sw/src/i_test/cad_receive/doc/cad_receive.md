# SW_INTEGRATION_TEST_2

## cad_receive overview

Validate persistence of a LoRa CAD operation across clear scans and a receive-on-detection packet cycle.

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

From confirmed idle, the host configures LoRa receive-on-detection CAD. A clear scan, subsequent detection and received packet exercise the [persistent CAD contract](../../../../../sys/req/sys_interfaces.md#cad-operation); detection and packet delivery are reported while the functional operation remains CAD.

## Verification scope

Evidence covers configuration event fields, an empty CAD_DETECTED report payload, packet bytes/metadata in the event and framed host report, and diagnostics with zero errors. Real host/RF active objects and EDF run with simulated radio indications at the HAL boundary. These observations do not prove physical scanning or automatic reception, and extra events/reports are not exhaustively detected. Terminal detection and timeout outcomes belong to [cad_exits](../../cad_exits/doc/cad_exits.md). Host target: `i_test_cad_receive`.
