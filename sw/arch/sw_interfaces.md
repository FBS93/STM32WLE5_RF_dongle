# sw_interfaces

## SW_ARCH_INTERFACE_1

Name: swi_hostUsbControl

Interface inherited from upstream definition.

Upstream traceability:
- [SW_REQ_INTERFACE_1](../req/sw_interfaces.md#sw_req_interface_1)

Status: approved

## SW_ARCH_INTERFACE_2

Name: swi_rfInputOutput

Interface inherited from upstream definition.

Upstream traceability:
- [SW_REQ_INTERFACE_2](../req/sw_interfaces.md#sw_req_interface_2)

Status: approved

## SW_ARCH_INTERFACE_4

Name: swi_hostCommandManagerEdf

The interface connects [`swc_hostCommandManager`](sw_components.md#sw_arch_component_2) to [`swc_edf`](sw_components.md#sw_arch_component_5). It publishes host protocol request events and subscribes to RF response/report events.

Published events:
- GET_CONFIG_REQUEST
- SET_CONFIG
- CALIBRATE
- TX_PACKET
- RX_START
- CAD_START
- TX_CONTINUOUS_WAVE
- TX_CONTINUOUS_PREAMBLE
- STOP
- GET_DIAGNOSTICS_REQUEST

Subscribed events:
- GET_CONFIG_RESPONSE
- GET_DIAGNOSTICS_RESPONSE
- RX_PACKET
- CAD_DETECTED

Upstream traceability:
- [SW_REQ_CONTROLLER_1](../req/sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_1](../req/sw_interfaces.md#sw_req_interface_1)

Status: approved

## SW_ARCH_INTERFACE_5

Name: swi_rfManagerEdf

The interface connects [`swc_rfManager`](sw_components.md#sw_arch_component_3) to [`swc_edf`](sw_components.md#sw_arch_component_5). It subscribes to host protocol request events and publishes RF response/report events.

Published events:
- GET_CONFIG_RESPONSE
- GET_DIAGNOSTICS_RESPONSE
- RX_PACKET
- CAD_DETECTED

Subscribed events:
- GET_CONFIG_REQUEST
- SET_CONFIG
- CALIBRATE
- TX_PACKET
- RX_START
- CAD_START
- TX_CONTINUOUS_WAVE
- TX_CONTINUOUS_PREAMBLE
- STOP
- GET_DIAGNOSTICS_REQUEST

Upstream traceability:
- [SW_REQ_CONTROLLER_1](../req/sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_2](../req/sw_interfaces.md#sw_req_interface_2)

Status: approved
