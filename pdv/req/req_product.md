# Product requirements

# REQ_PRODUCT_1

`STM32WLE5_RF_dongle` shall implement the communication protocol defined by the [reqi_hostUsbControl](req_interfaces.md#req_interface_1) interface, support all parameters defined by the associated parameter requirements, and perform RF receive and transmit operations through [reqi_rfInputOutput](req_interfaces.md#req_interface_2).

Validation criteria:
- Exercise all protocol commands with representative and boundary-valid cases covering all parameters and verify protocol conformance.
- Exercise RX  and TX operations and verify that RF signals are received from and delivered to the external environment through [reqi_rfInputOutput](req_interfaces.md#req_interface_2).

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_2](../spec/specs.md#spec_2)
- [SPEC_3](../spec/specs.md#spec_3)
- [SPEC_4](../spec/specs.md#spec_4)
- [SPEC_5](../spec/specs.md#spec_5)
- [SPEC_6](../spec/specs.md#spec_6)

References:
- [REQ_INTERFACE_1](req_interfaces.md#req_interface_1)
- [REQ_INTERFACE_2](req_interfaces.md#req_interface_2)
- [REQ_PARAMETER_1](req_parameters.md#req_parameter_1)
- [REQ_PARAMETER_2](req_parameters.md#req_parameter_2)
- [REQ_PARAMETER_3](req_parameters.md#req_parameter_3)
- [REQ_PARAMETER_4](req_parameters.md#req_parameter_4)
- [REQ_PARAMETER_5](req_parameters.md#req_parameter_5)
- [REQ_PARAMETER_6](req_parameters.md#req_parameter_6)
- [REQ_PARAMETER_7](req_parameters.md#req_parameter_7)
- [REQ_PARAMETER_8](req_parameters.md#req_parameter_8)
- [REQ_PARAMETER_9](req_parameters.md#req_parameter_9)
- [REQ_PARAMETER_10](req_parameters.md#req_parameter_10)
- [REQ_PARAMETER_11](req_parameters.md#req_parameter_11)
- [REQ_PARAMETER_12](req_parameters.md#req_parameter_12)
- [REQ_PARAMETER_13](req_parameters.md#req_parameter_13)
- [REQ_PARAMETER_14](req_parameters.md#req_parameter_14)
- [REQ_PARAMETER_15](req_parameters.md#req_parameter_15)
- [REQ_PARAMETER_16](req_parameters.md#req_parameter_16)
- [REQ_PARAMETER_17](req_parameters.md#req_parameter_17)
- [REQ_PARAMETER_18](req_parameters.md#req_parameter_18)
- [REQ_PARAMETER_19](req_parameters.md#req_parameter_19)
- [REQ_PARAMETER_20](req_parameters.md#req_parameter_20)
- [REQ_PARAMETER_21](req_parameters.md#req_parameter_21)
- [REQ_PARAMETER_22](req_parameters.md#req_parameter_22)
- [REQ_PARAMETER_23](req_parameters.md#req_parameter_23)
- [REQ_PARAMETER_24](req_parameters.md#req_parameter_24)
- [REQ_PARAMETER_25](req_parameters.md#req_parameter_25)
- [REQ_PARAMETER_26](req_parameters.md#req_parameter_26)
- [REQ_PARAMETER_27](req_parameters.md#req_parameter_27)
- [REQ_PARAMETER_28](req_parameters.md#req_parameter_28)
- [REQ_PARAMETER_29](req_parameters.md#req_parameter_29)
- [REQ_PARAMETER_30](req_parameters.md#req_parameter_30)
- [REQ_PARAMETER_31](req_parameters.md#req_parameter_31)
- [REQ_PARAMETER_32](req_parameters.md#req_parameter_32)
- [REQ_PARAMETER_33](req_parameters.md#req_parameter_33)
- [REQ_PARAMETER_34](req_parameters.md#req_parameter_34)
- [REQ_PARAMETER_35](req_parameters.md#req_parameter_35)
- [REQ_PARAMETER_36](req_parameters.md#req_parameter_36)
- [REQ_PARAMETER_37](req_parameters.md#req_parameter_37)
- [REQ_PARAMETER_38](req_parameters.md#req_parameter_38)
- [REQ_PARAMETER_39](req_parameters.md#req_parameter_39)
- [REQ_PARAMETER_40](req_parameters.md#req_parameter_40)
- [REQ_PARAMETER_41](req_parameters.md#req_parameter_41)
- [REQ_PARAMETER_42](req_parameters.md#req_parameter_42)
- [REQ_PARAMETER_43](req_parameters.md#req_parameter_43)
- [REQ_PARAMETER_44](req_parameters.md#req_parameter_44)
- [REQ_PARAMETER_45](req_parameters.md#req_parameter_45)
- [REQ_PARAMETER_46](req_parameters.md#req_parameter_46)
- [REQ_PARAMETER_47](req_parameters.md#req_parameter_47)

Status: approved
