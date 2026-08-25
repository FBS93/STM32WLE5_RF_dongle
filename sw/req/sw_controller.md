# Software controller requirements

# SW_REQ_CONTROLLER_1

The software shall implement the communication protocol defined by [swi_hostUsbControl](sw_interfaces.md#sw_req_interface_1), support all software parameters defined by [SW_REQ_PARAMETER_1](sw_parameters.md#sw_req_parameter_1), and perform RF receive and transmit operations through [swi_rfInputOutput](sw_interfaces.md#sw_req_interface_2).

Validation criteria:
- Exercise all protocol commands with representative and boundary-valid cases covering all software parameters and verify protocol conformance.
- Exercise RX and TX operations and verify that RF signals are received from and delivered to the external environment through [swi_rfInputOutput](sw_interfaces.md#sw_req_interface_2).

Upstream traceability:
- [SYS_REQ_PRODUCT_1](../../sys/req/sys_product.md#sys_req_product_1)
- [SYS_REQ_INTERFACE_1](../../sys/req/sys_interfaces.md#sys_req_interface_1)
- [SYS_REQ_INTERFACE_2](../../sys/req/sys_interfaces.md#sys_req_interface_2)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_DESIGN_1](../../sys/arch/sys_designs.md#sys_arch_design_1)

References:
- [SW_REQ_INTERFACE_1](sw_interfaces.md#sw_req_interface_1)
- [SW_REQ_INTERFACE_2](sw_interfaces.md#sw_req_interface_2)
- [SW_REQ_PARAMETER_1](sw_parameters.md#sw_req_parameter_1)
- [SW_REQ_PLATFORM_1](sw_platform.md#sw_req_platform_1)
- [SW_REQ_PLATFORM_2](sw_platform.md#sw_req_platform_2)
- [SW_REQ_PLATFORM_3](sw_platform.md#sw_req_platform_3)
- [SW_REQ_PLATFORM_4](sw_platform.md#sw_req_platform_4)
- [SW_REQ_PLATFORM_5](sw_platform.md#sw_req_platform_5)
- [SW_REQ_PLATFORM_6](sw_platform.md#sw_req_platform_6)
- [SW_REQ_PLATFORM_7](sw_platform.md#sw_req_platform_7)

Status: approved
