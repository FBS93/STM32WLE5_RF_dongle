# sw_components

## SW_ARCH_COMPONENT_1

Name: swc_hostCli

Host-side software component. It accepts the user command line and maps the CLI behavior to the target protocol requests and responses.

The component uses [`swi_hostUsbControl`](sw_interfaces.md#sw_arch_interface_1), as the logical host-control boundary to the target deployment.

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

References:
- [SW_ARCH_INTERFACE_1](sw_interfaces.md#sw_arch_interface_1)

Status: approved

## SW_ARCH_COMPONENT_2

Name: swc_hostCommandManager

Target-side active object responsible for the host-control boundary. It owns and realizes [`swi_hostUsbControl`](sw_interfaces.md#sw_arch_interface_1) and uses [`swi_hostCommandManagerEdf`](sw_interfaces.md#sw_arch_interface_4) as its single EDF architecture interface: it publishes accepted host-control requests and subscribes to target result events, exposing back those results through [`swi_hostUsbControl`](sw_interfaces.md#sw_arch_interface_1).

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_COMPONENT_2](../../sys/arch/sys_components.md#sys_arch_component_2)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)
- [REQ_INTERFACE_1](../../pdv/req/req_interfaces.md#req_interface_1)
- [HW_SW_INTERFACE_7](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_7)

References:
- [SW_ARCH_INTERFACE_1](sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_4](sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_COMPONENT_5](#sw_arch_component_5)
- [SW_ARCH_DESIGN_1](sw_designs.md#sw_arch_design_1)
- [SW_ARCH_DESIGN_2](sw_designs.md#sw_arch_design_2)

Status: approved

## SW_ARCH_COMPONENT_3

Name: swc_rfManager

Target-side active object responsible for RF operations. It owns and realizes [`swi_rfInputOutput`](sw_interfaces.md#sw_arch_interface_2) and uses [`swi_rfManagerEdf`](sw_interfaces.md#sw_arch_interface_5) as its single EDF architecture interface: it subscribes to operation and configuration RF events, applies the RF semantic parameters from [`SW_ARCH_PARAMETER_1`](sw_parameters.md#sw_arch_parameter_1), and publishes RF result events.

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)
- [HW_SW_INTERFACE_9](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9)
- [HW_SW_INTERFACE_12](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)
- [REQ_INTERFACE_2](../../pdv/req/req_interfaces.md#req_interface_2)
- [HW_SW_INTERFACE_10](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_10)
- [REQ_PARAMETER_35](../../pdv/req/req_parameters.md#req_parameter_35)

References:
- [SW_ARCH_INTERFACE_2](sw_interfaces.md#sw_arch_interface_2)
- [SW_ARCH_INTERFACE_5](sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_PARAMETER_1](sw_parameters.md#sw_arch_parameter_1)
- [SW_ARCH_COMPONENT_5](#sw_arch_component_5)
- [SW_ARCH_DESIGN_1](sw_designs.md#sw_arch_design_1)
- [SW_ARCH_DESIGN_2](sw_designs.md#sw_arch_design_2)

Status: approved

## SW_ARCH_COMPONENT_5

Name: swc_edf

Target-side reusable [Event Driven Framework (EDF)](../ecf/event_driven_framework/doc/edf.md) component. It provides the event-driven publish/subscribe mechanism used by the target active objects.

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)

References:
- [SW_ARCH_DESIGN_1](sw_designs.md#sw_arch_design_1)
- [SW_ARCH_INTERFACE_4](sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](sw_interfaces.md#sw_arch_interface_5)

Status: approved
