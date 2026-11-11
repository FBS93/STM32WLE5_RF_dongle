# SW_INTEGRATION_TEST_9

## tx_continuous overview

Validate continuous-wave and infinite-preamble TX persistence, replacement and explicit termination.

Upstream traceability:
- [SW_ARCH_INTERFACE_1](../../../../arch/sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_4](../../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](../../../../arch/sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_DESIGN_1](../../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_PARAMETER_1](../../../../arch/sw_parameters.md#sw_arch_parameter_1)

Status: approved

## Use case

From confirmed idle and a complete LoRa configuration, continuous-wave TX persists across diagnostic queries. An infinite-preamble request replaces it directly and remains active until STOP, exercising the [replacement contract](../../../../../sys/req/sys_interfaces.md#command-processing) without a radio completion indication.

## Verification scope

Evidence covers request events, distinct wave/preamble identities and zero errors in diagnostic events/reports, command-only silence and final idle. Real host/RF active objects and EDF run with HAL doubles; these observations do not establish RF waveform correctness, uninterrupted transmission or HAL stop/start ordering. Extra events/reports are not exhaustively detected. Host target: `i_test_tx_continuous`.
