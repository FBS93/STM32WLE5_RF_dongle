# sys_components

## SYS_ARCH_COMPONENT_1

Name: sysc_controller

`Controller` is the indivisible system abstraction of the MCU hardware together with the software executing on it. It owns the product-side host-control endpoint, applies the RF configuration and semantic parameter set, executes the requested RF operations, emits the defined responses, and exposes the host-visible operation state and radio errors. MCU implementation details and any RF-front-end partitioning are intentionally below this system-architecture level.

Upstream traceability:
- [SYS_REQ_PRODUCT_1](../req/sys_product.md#sys_req_product_1)
- [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#sys_req_interface_1)
- [SYS_REQ_INTERFACE_2](../req/sys_interfaces.md#sys_req_interface_2)

References:
- [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](sys_designs.md#sys_arch_design_1)
- [SYS_ARCH_PARAMETER_1](sys_parameters.md#sys_arch_parameter_1)
- [SYS_ARCH_PARAMETER_2](sys_parameters.md#sys_arch_parameter_2)
- [SYS_ARCH_PARAMETER_3](sys_parameters.md#sys_arch_parameter_3)
- [SYS_ARCH_PARAMETER_4](sys_parameters.md#sys_arch_parameter_4)
- [SYS_ARCH_PARAMETER_5](sys_parameters.md#sys_arch_parameter_5)
- [SYS_ARCH_PARAMETER_6](sys_parameters.md#sys_arch_parameter_6)
- [SYS_ARCH_PARAMETER_7](sys_parameters.md#sys_arch_parameter_7)
- [SYS_ARCH_PARAMETER_8](sys_parameters.md#sys_arch_parameter_8)
- [SYS_ARCH_PARAMETER_9](sys_parameters.md#sys_arch_parameter_9)
- [SYS_ARCH_PARAMETER_10](sys_parameters.md#sys_arch_parameter_10)
- [SYS_ARCH_PARAMETER_11](sys_parameters.md#sys_arch_parameter_11)
- [SYS_ARCH_PARAMETER_12](sys_parameters.md#sys_arch_parameter_12)
- [SYS_ARCH_PARAMETER_13](sys_parameters.md#sys_arch_parameter_13)
- [SYS_ARCH_PARAMETER_14](sys_parameters.md#sys_arch_parameter_14)
- [SYS_ARCH_PARAMETER_15](sys_parameters.md#sys_arch_parameter_15)
- [SYS_ARCH_PARAMETER_16](sys_parameters.md#sys_arch_parameter_16)
- [SYS_ARCH_PARAMETER_17](sys_parameters.md#sys_arch_parameter_17)
- [SYS_ARCH_PARAMETER_18](sys_parameters.md#sys_arch_parameter_18)
- [SYS_ARCH_PARAMETER_19](sys_parameters.md#sys_arch_parameter_19)
- [SYS_ARCH_PARAMETER_20](sys_parameters.md#sys_arch_parameter_20)
- [SYS_ARCH_PARAMETER_21](sys_parameters.md#sys_arch_parameter_21)
- [SYS_ARCH_PARAMETER_22](sys_parameters.md#sys_arch_parameter_22)
- [SYS_ARCH_PARAMETER_23](sys_parameters.md#sys_arch_parameter_23)
- [SYS_ARCH_PARAMETER_24](sys_parameters.md#sys_arch_parameter_24)
- [SYS_ARCH_PARAMETER_25](sys_parameters.md#sys_arch_parameter_25)
- [SYS_ARCH_PARAMETER_26](sys_parameters.md#sys_arch_parameter_26)
- [SYS_ARCH_PARAMETER_27](sys_parameters.md#sys_arch_parameter_27)
- [SYS_ARCH_PARAMETER_28](sys_parameters.md#sys_arch_parameter_28)
- [SYS_ARCH_PARAMETER_29](sys_parameters.md#sys_arch_parameter_29)
- [SYS_ARCH_PARAMETER_30](sys_parameters.md#sys_arch_parameter_30)
- [SYS_ARCH_PARAMETER_31](sys_parameters.md#sys_arch_parameter_31)
- [SYS_ARCH_PARAMETER_32](sys_parameters.md#sys_arch_parameter_32)
- [SYS_ARCH_PARAMETER_33](sys_parameters.md#sys_arch_parameter_33)
- [SYS_ARCH_PARAMETER_34](sys_parameters.md#sys_arch_parameter_34)
- [SYS_ARCH_PARAMETER_35](sys_parameters.md#sys_arch_parameter_35)
- [SYS_ARCH_PARAMETER_36](sys_parameters.md#sys_arch_parameter_36)
- [SYS_ARCH_PARAMETER_37](sys_parameters.md#sys_arch_parameter_37)
- [SYS_ARCH_PARAMETER_38](sys_parameters.md#sys_arch_parameter_38)
- [SYS_ARCH_PARAMETER_39](sys_parameters.md#sys_arch_parameter_39)
- [SYS_ARCH_PARAMETER_40](sys_parameters.md#sys_arch_parameter_40)
- [SYS_ARCH_PARAMETER_41](sys_parameters.md#sys_arch_parameter_41)
- [SYS_ARCH_PARAMETER_42](sys_parameters.md#sys_arch_parameter_42)
- [SYS_ARCH_PARAMETER_43](sys_parameters.md#sys_arch_parameter_43)
- [SYS_ARCH_PARAMETER_44](sys_parameters.md#sys_arch_parameter_44)
- [SYS_ARCH_PARAMETER_45](sys_parameters.md#sys_arch_parameter_45)
- [SYS_ARCH_PARAMETER_46](sys_parameters.md#sys_arch_parameter_46)
- [SYS_ARCH_PARAMETER_47](sys_parameters.md#sys_arch_parameter_47)

Status: approved

## SYS_ARCH_COMPONENT_2

Name: sysc_hostCli

Software component executing on the host that provides the CLI and uses `sysi_usbExternal` to communicate with `sysc_controller`.

Upstream traceability:
- [SYS_REQ_CLI_1](../req/sys_cli.md#sys_req_cli_1)
- [SYS_REQ_CLI_2](../req/sys_cli.md#sys_req_cli_2)
- [SYS_REQ_CLI_3](../req/sys_cli.md#sys_req_cli_3)
- [SYS_REQ_CLI_4](../req/sys_cli.md#sys_req_cli_4)
- [SYS_REQ_CLI_5](../req/sys_cli.md#sys_req_cli_5)
- [SYS_REQ_CLI_6](../req/sys_cli.md#sys_req_cli_6)
- [SYS_REQ_CLI_7](../req/sys_cli.md#sys_req_cli_7)

References:
- [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1)

Status: approved
