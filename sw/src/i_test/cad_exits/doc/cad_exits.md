# SW_INTEGRATION_TEST_8

## cad_exits overview

Validate the distinct terminal and persistent CAD detection outcomes.

Upstream traceability:
- [SW_ARCH_INTERFACE_1](../../../../arch/sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_4](../../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](../../../../arch/sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_DESIGN_1](../../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_PARAMETER_1](../../../../arch/sw_parameters.md#sw_arch_parameter_1)

Status: approved

## Use case

Two scenarios use real host/RF active objects and EDF with HAL doubles initialized once. Each scenario establishes its own functional preconditions through STOP, idle diagnostics and a complete configuration; EDFTest phases remain sequential within each scenario, without claiming full process isolation.

- Stop-on-detection (six phases): STOP, idle diagnostics, complete stop-on-detection configuration, CAD start, registered detection with `CAD_DETECTED` report, then idle diagnostics.
- Receive-on-detection (ten phases): STOP, idle diagnostics, complete receive-on-detection configuration, CAD start, registered detection with `CAD_DETECTED` report, CAD diagnostics, registered RX timeout followed by CAD diagnostics, a second detection with another report, STOP, then idle diagnostics.

## Verification scope

Check configuration event exit mode, detection events and empty host report payloads, CAD diagnostics after receive timeout, and idle after stop-on-detection or STOP. Oracle: [inherited CAD contract](../../../../../sys/req/sys_interfaces.md#cad-operation). HAL call counts/order, rearming, configuration handling and detection/completion ordering are not checked; extra events/reports are not exhaustively detected. Diagnostics and injected indications do not prove resumed physical scanning. Clear-scan and packet delivery scenarios are covered by [cad_receive](../../cad_receive/doc/cad_receive.md). Host target: `i_test_cad_exits`.
