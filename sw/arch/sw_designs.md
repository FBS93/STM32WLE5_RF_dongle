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
  - Description: Represents the [GET_CONFIG_REQUEST](../../pdv/req/req_interfaces.md#get-config-request) command.
- `SET_CONFIG`
  - Description: Represents the [SET_CONFIG](../../pdv/req/req_interfaces.md#set-config-request) command.
  - Parameters:
    - `configuration_payload`: [Configuration payload](../../pdv/req/req_interfaces.md#configuration-payload).
- `CALIBRATE`
  - Description: Represents the [CALIBRATE](../../pdv/req/req_interfaces.md#calibrate-request) command.
  - Parameters:
    - `calibration_payload`: [Calibration payload](../../pdv/req/req_interfaces.md#calibration-payload).
- `TX_PACKET`
  - Description: Represents the [TX_PACKET](../../pdv/req/req_interfaces.md#tx-packet-request) command.
  - Parameters:
    - `tx_packet_payload`: [TX Packet payload](../../pdv/req/req_interfaces.md#tx-packet-payload).
    - `payload_length`: [TX packet payload length](../../pdv/req/req_interfaces.md#tx-packet-payload).
- `RX_START`
  - Description: Represents the [RX_START](../../pdv/req/req_interfaces.md#rx-start-request) command.
- `CAD_START`
  - Description: Represents the [CAD_START](../../pdv/req/req_interfaces.md#cad-start-request) command.
- `TX_CONTINUOUS_WAVE`
  - Description: Represents the [TX_CONTINUOUS_WAVE](../../pdv/req/req_interfaces.md#tx-continuous-wave-request) command.
- `TX_CONTINUOUS_PREAMBLE`
  - Description: Represents the [TX_CONTINUOUS_PREAMBLE](../../pdv/req/req_interfaces.md#tx-continuous-preamble-request) command.
- `STOP`
  - Description: Represents the [STOP](../../pdv/req/req_interfaces.md#stop-request) command.
- `GET_DIAGNOSTICS_REQUEST`
  - Description: Represents the [GET_DIAGNOSTICS_REQUEST](../../pdv/req/req_interfaces.md#get-diagnostics-request) command.
- `GET_CONFIG_RESPONSE`
  - Description: Represents the [GET_CONFIG_RESPONSE](../../pdv/req/req_interfaces.md#get-config-response) command.
  - Parameters:
    - `configuration_payload`: [Configuration payload](../../pdv/req/req_interfaces.md#configuration-payload).
- `GET_DIAGNOSTICS_RESPONSE`
  - Description: Represents the [GET_DIAGNOSTICS_RESPONSE](../../pdv/req/req_interfaces.md#get-diagnostics-response) command.
  - Parameters:
    - `diagnostics_payload`: [Diagnostics payload](../../pdv/req/req_interfaces.md#diagnostics-payload).
- `RX_PACKET`
  - Description: Represents the [RX_PACKET](../../pdv/req/req_interfaces.md#rx-packet) command.
  - Parameters:
    - `rx_packet_payload`: [RX packet payload](../../pdv/req/req_interfaces.md#rx-packet-payload).
- `CAD_DETECTED`
  - Description: Represents the [CAD_DETECTED](../../pdv/req/req_interfaces.md#cad-detected) command.
- `OPERATION_COMPLETE`
  - Description: Indicates that the active RF operation has completed.
- `HOST_DATA_AVAILABLE`
  - Description: Indicates that data received from the host is available for processing.

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_COMPONENT_2](../../sys/arch/sys_components.md#sys_arch_component_2)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)
- [HW_SW_INTERFACE_7](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_7)
- [HW_SW_INTERFACE_9](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9)
- [HW_SW_INTERFACE_10](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_10)
- [HW_SW_INTERFACE_12](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12)
- [REQ_CLI_1](../../pdv/req/req_cli.md#req_cli_1)
- [REQ_CLI_2](../../pdv/req/req_cli.md#req_cli_2)
- [REQ_CLI_3](../../pdv/req/req_cli.md#req_cli_3)
- [REQ_CLI_4](../../pdv/req/req_cli.md#req_cli_4)
- [REQ_CLI_5](../../pdv/req/req_cli.md#req_cli_5)
- [REQ_CLI_6](../../pdv/req/req_cli.md#req_cli_6)
- [REQ_CLI_7](../../pdv/req/req_cli.md#req_cli_7)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)
- [REQ_INTERFACE_1](../../pdv/req/req_interfaces.md#req_interface_1)
- [REQ_INTERFACE_2](../../pdv/req/req_interfaces.md#req_interface_2)

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
- The project-specific platform package is selected with `ECF_TARGET_PLATFORM=stm32wle5c8u6_platform`. This CMake variable can also be used to select the appropriate source code for target and host environments.

ISR integration strategy:
The following table lists the interrupts from highest to lowest NVIC preemption priority; all use subpriority 0.

| Priority | Source | Owner | Purpose |
|---|---|---|---|
| 0 | `SUBGHZ_Radio_IRQn` | [`rfManager`](sw_components.md#sw_arch_component_3) | Report asynchronous radio indications. |
| 1 | `USART2_IRQn` | [`hostCommandManager`](sw_components.md#sw_arch_component_2) | Receive host bytes, publish `HOST_DATA_AVAILABLE`, and transmit host responses from buffered output. |

Interrupts that publish EDF events shall finish with `EDF_ISRExit()`. All event-producing interrupt sources shall remain disabled until the end of `EDF_onStartup()`.

RAM layout:
| Name | Description | Start address | End address |
|---|---|---|---|
| target_static_ram | 16 KiB available for data/BSS and unused space; section sizes are determined at link time. | 0x2000 8000 | 0x2000 BFFF |
| target_heap_none | No dynamic heap; 0 bytes reserved. | N/A | N/A |
| target_msp_stack | Fixed 4096-byte MSP stack reservation. | 0x2000 C000 | 0x2000 CFFF |

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
- The target ECF stdout shall use non-blocking buffered SEGGER RTT output over SWD, with 2048 bytes of usable buffer capacity. The debug probe shall drain the buffer; no target stdout interrupt is required.
- Target ECF stdin is not supported.

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)
- [REQ_INTERFACE_1](../../pdv/req/req_interfaces.md#req_interface_1)
- [REQ_INTERFACE_2](../../pdv/req/req_interfaces.md#req_interface_2)
- [REQ_PARAMETER_35](../../pdv/req/req_parameters.md#req_parameter_35)
- [HW_SW_INTERFACE_1](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_1)
- [HW_SW_INTERFACE_3](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_3)
- [HW_SW_INTERFACE_4](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_4)
- [HW_SW_INTERFACE_6](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_6)
- [HW_SW_INTERFACE_7](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_7)
- [HW_SW_INTERFACE_8](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_8)
- [HW_SW_INTERFACE_9](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9)
- [HW_SW_INTERFACE_10](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_10)
- [HW_SW_INTERFACE_12](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12)

Status: approved

## SW_ARCH_DESIGN_3

Name: swd_hostCliScope

The [swc_hostCli](sw_components.md#sw_arch_component_1) shall be implemented as a Python 3 application for POSIX. It is defined up to software architecture only; downstream software-domain processes do not apply. The [swc_hostCli](sw_components.md#sw_arch_component_1) remains subject to system integration and qualification testing.

Upstream traceability:
- [SYS_ARCH_COMPONENT_2](../../sys/arch/sys_components.md#sys_arch_component_2)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [REQ_CLI_1](../../pdv/req/req_cli.md#req_cli_1)
- [REQ_CLI_2](../../pdv/req/req_cli.md#req_cli_2)
- [REQ_CLI_3](../../pdv/req/req_cli.md#req_cli_3)
- [REQ_CLI_4](../../pdv/req/req_cli.md#req_cli_4)
- [REQ_CLI_5](../../pdv/req/req_cli.md#req_cli_5)
- [REQ_CLI_6](../../pdv/req/req_cli.md#req_cli_6)
- [REQ_CLI_7](../../pdv/req/req_cli.md#req_cli_7)

Status: approved
