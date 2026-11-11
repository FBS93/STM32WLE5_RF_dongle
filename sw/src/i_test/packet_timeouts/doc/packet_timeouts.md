# SW_INTEGRATION_TEST_6

## packet_timeouts overview

Validate callback-driven finalization of single RX and packet TX without packet delivery or a host completion message.

Upstream traceability:
- [SW_ARCH_INTERFACE_1](../../../../arch/sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_4](../../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](../../../../arch/sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_DESIGN_1](../../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_PARAMETER_1](../../../../arch/sw_parameters.md#sw_arch_parameter_1)

Status: approved

## Use case

Each scenario establishes confirmed idle and its own complete LoRa configuration with finite timeouts:

- Single RX ends on a simulated receive timeout, without delivering a packet, as required by the [single-RX contract](../../../../../sys/req/sys_interfaces.md#rx-operation).
- Packet TX ends on a simulated transmit timeout rather than a successful completion indication, following [TX timeout semantics](../../../../../sys/req/sys_parameters.md#sys_req_parameter_37).

## Verification scope

Evidence distinguishes active-operation and idle diagnostics with zero errors, observes OPERATION_COMPLETE and checks no host output during completion. Real host/RF active objects and EDF run with HAL doubles; this verifies indication processing, not elapsed radio timing. Extra events/reports are not exhaustively detected, and functional preparation does not imply full process isolation. Host target: `i_test_packet_timeouts`.
