# sw_interfaces

## SW_ARCH_INTERFACE_1

Name: swi_hostUsbControl

Interface inherited from upstream definition.

Upstream traceability:
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)

Status: approved

## SW_ARCH_INTERFACE_2

Name: swi_rfInputOutput

Interface inherited from upstream definition.

Upstream traceability:
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)

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
- HOST_DATA_AVAILABLE

Subscribed events:
- GET_CONFIG_RESPONSE
- GET_DIAGNOSTICS_RESPONSE
- RX_PACKET
- CAD_DETECTED
- HOST_DATA_AVAILABLE

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_COMPONENT_2](../../sys/arch/sys_components.md#sys_arch_component_2)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)
- [HW_SW_INTERFACE_7](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_7)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)
- [REQ_INTERFACE_1](../../pdv/req/req_interfaces.md#req_interface_1)

Status: approved

## SW_ARCH_INTERFACE_5

Name: swi_rfManagerEdf

The interface connects [`swc_rfManager`](sw_components.md#sw_arch_component_3) to [`swc_edf`](sw_components.md#sw_arch_component_5). It subscribes to host protocol request events and publishes RF response/report events.

Published events:
- GET_CONFIG_RESPONSE
- GET_DIAGNOSTICS_RESPONSE
- RX_PACKET
- CAD_DETECTED
- OPERATION_COMPLETE

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
- OPERATION_COMPLETE

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)
- [HW_SW_INTERFACE_9](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9)
- [HW_SW_INTERFACE_10](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_10)
- [HW_SW_INTERFACE_12](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12)
- [REQ_PRODUCT_1](../../pdv/req/req_product.md#req_product_1)
- [REQ_INTERFACE_2](../../pdv/req/req_interfaces.md#req_interface_2)

Status: approved
