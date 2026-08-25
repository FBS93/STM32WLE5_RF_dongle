# sw_components

## SW_ARCH_COMPONENT_1

Name: swc_hostCli

Host-side software component. It accepts the user command line and maps the CLI behavior to the target protocol requests and responses.

The component uses [`swi_hostUsbControl`](sw_interfaces.md#sw_arch_interface_1), as the logical host-control boundary to the target deployment.

Upstream traceability:
- [SW_REQ_CLI_1](../req/sw_cli.md#sw_req_cli_1)

References:
- [SW_ARCH_INTERFACE_1](sw_interfaces.md#sw_arch_interface_1)

Status: approved

## SW_ARCH_COMPONENT_2

Name: swc_hostCommandManager

Target-side active object responsible for the host-control boundary. It owns and realizes [`swi_hostUsbControl`](sw_interfaces.md#sw_arch_interface_1) and uses [`swi_hostCommandManagerEdf`](sw_interfaces.md#sw_arch_interface_4) as its single EDF architecture interface: it publishes accepted host-control requests and subscribes to target result events, exposing back those results through [`swi_hostUsbControl`](sw_interfaces.md#sw_arch_interface_1).

Upstream traceability:
- [SW_REQ_CONTROLLER_1](../req/sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_1](../req/sw_interfaces.md#sw_req_interface_1)
- [SW_REQ_PLATFORM_4](../req/sw_platform.md#sw_req_platform_4)

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
- [SW_REQ_CONTROLLER_1](../req/sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_2](../req/sw_interfaces.md#sw_req_interface_2)
- [SW_REQ_PLATFORM_6](../req/sw_platform.md#sw_req_platform_6)
- [SW_REQ_PLATFORM_7](../req/sw_platform.md#sw_req_platform_7)

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
- [SW_REQ_CONTROLLER_1](../req/sw_controller.md#sw_req_controller_1)

References:
- [SW_ARCH_DESIGN_1](sw_designs.md#sw_arch_design_1)
- [SW_ARCH_INTERFACE_4](sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](sw_interfaces.md#sw_arch_interface_5)

Status: approved
