# sw_designs

## SW_ARCH_DESIGN_1

Name: swd_eventDrivenModel

This architecture shall follow an event-driven model using Event Driven Framework (EDF).

Active-object priorities:
- EDF priority 1 is reserved for the [EDF Test AO](../ecf/event_driven_framework/edf_test/doc/edf_test.md).
- `hostCommandManager`: EDF priority 2.
- `rfManager`: EDF priority 3, prioritizing asynchronous RF indications over buffered host-command processing.
- Preemption thresholds are not used; each active object's threshold shall equal its base priority.

EDF resource decisions:
- Mutable events shall use EDF event pools. Its block size shall accommodate the largest event, and its block count shall cover all mutable events that can concurrently remain queued or under dispatch.
- EDF time events are not used.

Events:
- `GET_CONFIG_REQUEST`
  - Description: Represents the [GET_CONFIG_REQUEST](../../sys/req/sys_interfaces.md#get-config-request) command.
- `SET_CONFIG`
  - Description: Represents the [SET_CONFIG](../../sys/req/sys_interfaces.md#set-config-request) command.
  - Parameters:
    - `configuration_payload`: [Configuration payload](../../sys/req/sys_interfaces.md#configuration-payload).
- `CALIBRATE`
  - Description: Represents the [CALIBRATE](../../sys/req/sys_interfaces.md#calibrate-request) command.
  - Parameters:
    - `calibration_payload`: [Calibration payload](../../sys/req/sys_interfaces.md#calibration-payload).
- `TX_PACKET`
  - Description: Represents the [TX_PACKET](../../sys/req/sys_interfaces.md#tx-packet-request) command.
  - Parameters:
    - `tx_packet_payload`: [TX Packet payload](../../sys/req/sys_interfaces.md#tx-packet-payload).
    - `payload_length`: [TX packet payload length](../../sys/req/sys_interfaces.md#tx-packet-payload).
- `RX_START`
  - Description: Represents the [RX_START](../../sys/req/sys_interfaces.md#rx-start-request) command.
- `CAD_START`
  - Description: Represents the [CAD_START](../../sys/req/sys_interfaces.md#cad-start-request) command.
- `TX_CONTINUOUS_WAVE`
  - Description: Represents the [TX_CONTINUOUS_WAVE](../../sys/req/sys_interfaces.md#tx-continuous-wave-request) command.
- `TX_CONTINUOUS_PREAMBLE`
  - Description: Represents the [TX_CONTINUOUS_PREAMBLE](../../sys/req/sys_interfaces.md#tx-continuous-preamble-request) command.
- `STOP`
  - Description: Represents the [STOP](../../sys/req/sys_interfaces.md#stop-request) command.
- `GET_DIAGNOSTICS_REQUEST`
  - Description: Represents the [GET_DIAGNOSTICS_REQUEST](../../sys/req/sys_interfaces.md#get-diagnostics-request) command.
- `GET_CONFIG_RESPONSE`
  - Description: Represents the [GET_CONFIG_RESPONSE](../../sys/req/sys_interfaces.md#get-config-response) command.
  - Parameters:
    - `configuration_payload`: [Configuration payload](../../sys/req/sys_interfaces.md#configuration-payload).
- `GET_DIAGNOSTICS_RESPONSE`
  - Description: Represents the [GET_DIAGNOSTICS_RESPONSE](../../sys/req/sys_interfaces.md#get-diagnostics-response) command.
  - Parameters:
    - `diagnostics_payload`: [Diagnostics payload](../../sys/req/sys_interfaces.md#diagnostics-payload).
- `RX_PACKET`
  - Description: Represents the [RX_PACKET](../../sys/req/sys_interfaces.md#rx-packet) command.
  - Parameters:
    - `rx_packet_payload`: [RX packet payload](../../sys/req/sys_interfaces.md#rx-packet-payload).
- `CAD_DETECTED`
  - Description: Represents the [CAD_DETECTED](../../sys/req/sys_interfaces.md#cad-detected) command.
- `OPERATION_COMPLETE`
  - Description: Indicates that the active RF operation has completed.
- `HOST_DATA_AVAILABLE`
  - Description: Indicates that data received from the host is available for processing.

Upstream traceability:
- [SW_REQ_CLI_1](../req/sw_cli.md#sw_req_cli_1)
- [SW_REQ_CONTROLLER_1](../req/sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_1](../req/sw_interfaces.md#sw_req_interface_1)
- [SW_REQ_INTERFACE_2](../req/sw_interfaces.md#sw_req_interface_2)

Status: approved

## SW_ARCH_DESIGN_2

Name: swd_platform

The platform shall be defined as follows:

Toolchains:
- Host SiL/emulation: use the existing [linux_gcc.cmake](../../tools/cmake/toolchains/linux_gcc.cmake) toolchain.
- Target executable: use the existing [arm_gcc.cmake](../../tools/cmake/toolchains/arm_gcc.cmake) toolchain.

Build configuration:
- The following elements are CMake configure presets. Each preset selects the build configuration, optimization level and debug information level. The common warning options `-Wall -Wextra -Wconversion -Werror` apply to the listed product presets through a base configuration.
  - Host SiL/emulation:
    - `host`: CMAKE_BUILD_TYPE `Release`, optimization `-O2` and debug information `-g2`.
    - `host_debug`: CMAKE_BUILD_TYPE `Debug`, optimization `-O0` and debug information `-g3`.
    - `host_hard_debug`: CMAKE_BUILD_TYPE `Debug`, optimization `-O0`, debug information `-g3` and generation of preprocessed files for inspection `-save-temps=obj -P`.
  - Target:
    - `target_stm32wle5c8u6`: CMAKE_BUILD_TYPE `Release`, optimization `-O2` and debug information `-g2`.
    - `target_stm32wle5c8u6_debug`: CMAKE_BUILD_TYPE `Debug`, optimization `-O0` and debug information `-g3`.
    - `target_stm32wle5c8u6_hard_debug`: CMAKE_BUILD_TYPE `Debug`, optimization `-O0`, debug information `-g3` and generation of preprocessed files for inspection `-save-temps=obj -P`.
- The project-specific platform package is selected with `PROJECT_TARGET_PLATFORM=stm32wle5c8u6_platform`. This CMake variable can also be used to select the appropriate source code for target and host environments.

ISR integration strategy:
The following table lists the interrupts from highest to lowest NVIC preemption priority; all use subpriority 0.

| Priority | Source | Owner | Purpose |
|---|---|---|---|
| 0 | `SUBGHZ_Radio_IRQn` | [`rfManager`](sw_components.md#sw_arch_component_3) | Report asynchronous radio indications. |
| 1 | `USART2_IRQn` | [`hostCommandManager`](sw_components.md#sw_arch_component_2) | Receive host bytes, publish `HOST_DATA_AVAILABLE`, and transmit host responses from buffered output. |
| 2 | `TIM2_IRQn` | STM32WLE5C8U6 platform stdout integration | Drain the non-blocking ECF stdout buffer periodically. |

Interrupts that publish EDF events shall finish with `EDF_ISRExit()`. All event-producing interrupt sources shall remain disabled until the end of `EDF_onStartup()`; the TIM2 stdout source may be enabled with the stdout integration because it does not publish EDF events.

RAM layout:
| Name | Description | Start address | End address |
|---|---|---|---|
| target_sram2 | STM32WLE5C8U6 SRAM2. The C8 density provides 20 Kbytes; RM0461 maps SRAM2 from 0x2000 8000. | 0x2000 8000 | 0x2000 CFFF |

ROM layout:
| Name | Description | Start address | End address |
|---|---|---|---|
| target_program_flash | STM32WLE5C8U6 user flash. The C8 density provides 64 Kbytes and RM0461 maps user flash from 0x0800 0000. | 0x0800 0000 | 0x0800 FFFF |

NVM layout:
| Name | Description | Start address | End address |
|---|---|---|---|
| target_nvm_none | No NVM region is currently used. | N/A | N/A |

Debug configuration:
- The target shall use SWD as its debug interface.

Embedded C Framework (ECF) integration:
- The target ECF stdout shall use the ITM/TPIU/SWO interface with the following configuration:
  - TPIU asynchronous output protocol: NRZ.
  - Nominal SWO bitrate: 2 Mbit/s.
  - The target ECF stdout shall use a non-blocking 1024-byte buffer.
- Target ECF stdin is not supported.

Upstream traceability:
- [SW_REQ_PLATFORM_1](../req/sw_platform.md#sw_req_platform_1)
- [SW_REQ_PLATFORM_2](../req/sw_platform.md#sw_req_platform_2)
- [SW_REQ_PLATFORM_3](../req/sw_platform.md#sw_req_platform_3)
- [SW_REQ_PLATFORM_4](../req/sw_platform.md#sw_req_platform_4)
- [SW_REQ_PLATFORM_5](../req/sw_platform.md#sw_req_platform_5)
- [SW_REQ_PLATFORM_6](../req/sw_platform.md#sw_req_platform_6)
- [SW_REQ_PLATFORM_7](../req/sw_platform.md#sw_req_platform_7)

Status: approved

## SW_ARCH_DESIGN_3

Name: swd_hostCliScope

The [swc_hostCli](sw_components.md#sw_arch_component_1) shall be implemented as a Python 3 application for POSIX. It is defined up to software architecture only; downstream software-domain processes do not apply. The [swc_hostCli](sw_components.md#sw_arch_component_1) remains subject to system integration and qualification testing.

Upstream traceability:
- [SW_REQ_CLI_1](../req/sw_cli.md#sw_req_cli_1)

Status: approved
