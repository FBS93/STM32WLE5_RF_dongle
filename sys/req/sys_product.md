# Product requirements

# SYS_REQ_PRODUCT_1

`STM32WLE5_RF_dongle` shall implement the communication protocol defined by the [sysi_hostUsbControl](sys_interfaces.md#sys_req_interface_1) interface, support all system parameters defined by the associated parameter requirements, and perform RF receive and transmit operations through [sysi_rfInputOutput](sys_interfaces.md#sys_req_interface_2).

Validation criteria:
- Exercise all protocol commands with representative and boundary-valid cases covering all parameters and verify protocol conformance.
- Exercise RX  and TX operations and verify that RF signals are received from and delivered to the external environment through [sysi_rfInputOutput](sys_interfaces.md#sys_req_interface_2).

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_2](../../spec/specs.md#spec_2)
- [SPEC_3](../../spec/specs.md#spec_3)
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)

References:
- [SYS_REQ_INTERFACE_1](sys_interfaces.md#sys_req_interface_1)
- [SYS_REQ_INTERFACE_2](sys_interfaces.md#sys_req_interface_2)
- [SYS_REQ_PARAMETER_1](sys_parameters.md#sys_req_parameter_1)
- [SYS_REQ_PARAMETER_2](sys_parameters.md#sys_req_parameter_2)
- [SYS_REQ_PARAMETER_3](sys_parameters.md#sys_req_parameter_3)
- [SYS_REQ_PARAMETER_4](sys_parameters.md#sys_req_parameter_4)
- [SYS_REQ_PARAMETER_5](sys_parameters.md#sys_req_parameter_5)
- [SYS_REQ_PARAMETER_6](sys_parameters.md#sys_req_parameter_6)
- [SYS_REQ_PARAMETER_7](sys_parameters.md#sys_req_parameter_7)
- [SYS_REQ_PARAMETER_8](sys_parameters.md#sys_req_parameter_8)
- [SYS_REQ_PARAMETER_9](sys_parameters.md#sys_req_parameter_9)
- [SYS_REQ_PARAMETER_10](sys_parameters.md#sys_req_parameter_10)
- [SYS_REQ_PARAMETER_11](sys_parameters.md#sys_req_parameter_11)
- [SYS_REQ_PARAMETER_12](sys_parameters.md#sys_req_parameter_12)
- [SYS_REQ_PARAMETER_13](sys_parameters.md#sys_req_parameter_13)
- [SYS_REQ_PARAMETER_14](sys_parameters.md#sys_req_parameter_14)
- [SYS_REQ_PARAMETER_15](sys_parameters.md#sys_req_parameter_15)
- [SYS_REQ_PARAMETER_16](sys_parameters.md#sys_req_parameter_16)
- [SYS_REQ_PARAMETER_17](sys_parameters.md#sys_req_parameter_17)
- [SYS_REQ_PARAMETER_18](sys_parameters.md#sys_req_parameter_18)
- [SYS_REQ_PARAMETER_19](sys_parameters.md#sys_req_parameter_19)
- [SYS_REQ_PARAMETER_20](sys_parameters.md#sys_req_parameter_20)
- [SYS_REQ_PARAMETER_21](sys_parameters.md#sys_req_parameter_21)
- [SYS_REQ_PARAMETER_22](sys_parameters.md#sys_req_parameter_22)
- [SYS_REQ_PARAMETER_23](sys_parameters.md#sys_req_parameter_23)
- [SYS_REQ_PARAMETER_24](sys_parameters.md#sys_req_parameter_24)
- [SYS_REQ_PARAMETER_25](sys_parameters.md#sys_req_parameter_25)
- [SYS_REQ_PARAMETER_26](sys_parameters.md#sys_req_parameter_26)
- [SYS_REQ_PARAMETER_27](sys_parameters.md#sys_req_parameter_27)
- [SYS_REQ_PARAMETER_28](sys_parameters.md#sys_req_parameter_28)
- [SYS_REQ_PARAMETER_29](sys_parameters.md#sys_req_parameter_29)
- [SYS_REQ_PARAMETER_30](sys_parameters.md#sys_req_parameter_30)
- [SYS_REQ_PARAMETER_31](sys_parameters.md#sys_req_parameter_31)
- [SYS_REQ_PARAMETER_32](sys_parameters.md#sys_req_parameter_32)
- [SYS_REQ_PARAMETER_33](sys_parameters.md#sys_req_parameter_33)
- [SYS_REQ_PARAMETER_34](sys_parameters.md#sys_req_parameter_34)
- [SYS_REQ_PARAMETER_35](sys_parameters.md#sys_req_parameter_35)
- [SYS_REQ_PARAMETER_36](sys_parameters.md#sys_req_parameter_36)
- [SYS_REQ_PARAMETER_37](sys_parameters.md#sys_req_parameter_37)
- [SYS_REQ_PARAMETER_38](sys_parameters.md#sys_req_parameter_38)
- [SYS_REQ_PARAMETER_39](sys_parameters.md#sys_req_parameter_39)
- [SYS_REQ_PARAMETER_40](sys_parameters.md#sys_req_parameter_40)
- [SYS_REQ_PARAMETER_41](sys_parameters.md#sys_req_parameter_41)
- [SYS_REQ_PARAMETER_42](sys_parameters.md#sys_req_parameter_42)
- [SYS_REQ_PARAMETER_43](sys_parameters.md#sys_req_parameter_43)
- [SYS_REQ_PARAMETER_44](sys_parameters.md#sys_req_parameter_44)
- [SYS_REQ_PARAMETER_45](sys_parameters.md#sys_req_parameter_45)
- [SYS_REQ_PARAMETER_46](sys_parameters.md#sys_req_parameter_46)
- [SYS_REQ_PARAMETER_47](sys_parameters.md#sys_req_parameter_47)

Status: approved
