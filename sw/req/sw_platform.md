# Software platform requirements

# SW_REQ_PLATFORM_1

The target software shall configure MCU clocking as required by the applicable hardware-software interface.

Validation criteria:
- Measure the configured SYSCLK and verify that the clock configuration supplies the dependencies identified by the traced hardware-software interface.

Upstream traceability:
- [SYS_REQ_PRODUCT_1](../../sys/req/sys_product.md#sys_req_product_1)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [HW_SW_INTERFACE_1](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_1)
- [HW_SW_INTERFACE_8](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_8)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)

Status: approved

# SW_REQ_PLATFORM_2

The target software shall preserve the reset and boot-selection signal reservations defined by the applicable hardware-software interface.

Validation criteria:
- Reset the target with the hardware boot selection set to each supported state and verify that the target starts in the corresponding boot mode.

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [HW_SW_INTERFACE_3](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_3)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)

Status: approved

# SW_REQ_PLATFORM_3

The target software shall preserve the debug-signal reservations defined by the applicable hardware-software interface.

Validation criteria:
- Connect a debugger to the target debug interface and verify that a debug session is established successfully with the target.

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [HW_SW_INTERFACE_4](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_4)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)

Status: approved

# SW_REQ_PLATFORM_4

The target software shall configure the USART2 according to the serial settings defined by the applicable hardware-software interface.

Validation criteria:
- Inspect and measure the USART2 configuration and verify conformance to the traced hardware-software interface.

Upstream traceability:
- [SYS_REQ_INTERFACE_1](../../sys/req/sys_interfaces.md#sys_req_interface_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [HW_SW_INTERFACE_7](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_7)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_1](sw_interfaces.md#sw_req_interface_1)

Status: approved

# SW_REQ_PLATFORM_5

The target software shall provide control of the user LED for debug and signaling.

Validation criteria:
- Turn the user LED on and off and visually verify that the LED responds correctly in each state.

Upstream traceability:
- [HW_SW_INTERFACE_6](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_6)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)

Status: approved

# SW_REQ_PLATFORM_6

The target software shall control the RF front-end switch according to the RF operation state.

Validation criteria:
- Inspect and measure `RF_SWITCH_CTRL` while the software is in idle, receiving, and transmitting, and verify low in idle and receive and high in transmit.

Upstream traceability:
- [SYS_REQ_INTERFACE_2](../../sys/req/sys_interfaces.md#sys_req_interface_2)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [HW_SW_INTERFACE_10](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_10)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_2](sw_interfaces.md#sw_req_interface_2)

Status: approved

# SW_REQ_PLATFORM_7

The target software shall configure RF transmission for the connected RF front-end path.

Validation criteria:
- Inspect the radio transmit configuration and exercise a representative transmission for conformance to [HW_SW_INTERFACE_12](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12).

Upstream traceability:
- [SYS_REQ_INTERFACE_2](../../sys/req/sys_interfaces.md#sys_req_interface_2)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [HW_SW_INTERFACE_9](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9)
- [HW_SW_INTERFACE_12](../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12)

References:
- [SW_REQ_CONTROLLER_1](sw_controller.md#sw_req_controller_1)
- [SW_REQ_INTERFACE_2](sw_interfaces.md#sw_req_interface_2)

Status: approved
