# SW_INTEGRATION_TEST_5

## calibration_completion overview

Validate completion generated after synchronous calibration and consumed through real EDF to return to idle.

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

From confirmed idle without prior RF configuration, the host requests calibration over a valid image-calibration interval using the [defined encoding](../../../../../sys/req/sys_interfaces.md#calibration-payload). Normal synchronous HAL return leads to completion and idle without a host completion message.

## Verification scope

Evidence covers the request interval in its event, real OPERATION_COMPLETE delivery, absence of immediate host output and final diagnostic event/report with zero errors. Real host/RF active objects and EDF run with HAL doubles; neither an intermediate active state nor actual radio calibration is demonstrated. Host target: `i_test_calibration_completion`.
