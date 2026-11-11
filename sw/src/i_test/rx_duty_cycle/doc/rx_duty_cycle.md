# SW_INTEGRATION_TEST_7

## rx_duty_cycle overview

Validate duty-cycle RX diagnostics and packet delivery across successful reception, timeout and packet rejection.

Upstream traceability:
- [SW_ARCH_INTERFACE_1](../../../../arch/sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_4](../../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](../../../../arch/sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_DESIGN_1](../../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_PARAMETER_1](../../../../arch/sw_parameters.md#sw_arch_parameter_1)

Status: approved

## Use case

From confirmed idle and a complete LoRa duty-cycle configuration, reception delivers packets despite intervening timeout and rejection indications, until explicitly stopped. The scenario combines [RX persistence](../../../../../sys/req/sys_interfaces.md#rx-operation) with [duty-cycle semantics](../../../../../sys/req/sys_parameters.md#sys_req_parameter_32).

## Verification scope

Evidence covers configured mode/periods, bytes/metadata in both packet events and host reports, and RX/idle diagnostics with zero errors. Real host/RF active objects and EDF run with HAL doubles; injected indications do not prove physical duty-cycle timing or radio rearming. Extra events/reports are not exhaustively detected. Host target: `i_test_rx_duty_cycle`.
