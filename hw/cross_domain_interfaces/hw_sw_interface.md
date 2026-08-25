# hw_sw_interface

## MCU generic configuration

### HW_SW_INTERFACE_1

Software shall set SYSCLK to 32 MHz SYSCLK. USART2 and SUBGHZSPI depend on this clock. The configuration procedure shall follow RM0461.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (U1).

Upstream traceability:
- [SYS_REQ_PRODUCT_1](../../sys/req/sys_product.md#sys_req_product_1)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 3.3.4, 5.1.4, 6.2 and 6.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapter 3.12

Status: approved

### HW_SW_INTERFACE_3

Software shall not drive `NRST` or control `PH3/BOOT0`. Hardware samples `PH3/BOOT0` during reset/startup for boot selection.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (U1 `NRST` and `PH3/BOOT0`, boot jumper J1 and pull-down R1).

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 2.2, 5 and 8
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.7, 3.14 and 5.3.16

Status: approved

### HW_SW_INTERFACE_4

Software shall preserve PA13/AF0, PA14/AF0 and PB3/AF0 for the SWDIO, SWCLK and SWO functions, respectively.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (U1 `PA13`, `PA14` and `PB3`, SWD connector J2).

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapter 8
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.14 and 5.3.16

Status: approved

### HW_SW_INTERFACE_5

The TCXO supply-control connection is `PB0/VDDTCXO`.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (U1 PB0/VDDTCXO and Y1).

Upstream traceability:
- [SYS_REQ_PRODUCT_1](../../sys/req/sys_product.md#sys_req_product_1)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 3.8, 5, and 6
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.12 and 5.3.16

Status: approved

### HW_SW_INTERFACE_6

The user LED is connected to PA7; low asserts the LED and high deasserts it.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (U1 `PA7`, D1 and series resistor R2).

Upstream traceability:
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapter 8
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.14 and 5.3.16

Status: approved

## USB interface

### HW_SW_INTERFACE_7

FT230XS-R U4 `TXD` is connected to PA3/`USART2_RX`, and PA2/`USART2_TX` is connected to FT230XS-R `RXD`, through the `USB_interface_tx` and `USB_interface_rx` hierarchical nets. The interface uses USART2 at `115200` baud with 8 data bits, no parity, and 1 stop bit (8-N-1).

Hardware design reference: [top-level schematic](../../hw/KiCad/top.kicad_sch) and [USB interface schematic](../../hw/KiCad/USB_interface_and_power.kicad_sch) (U4 FT230XS-R, `TXD`, `RXD`, `USB_interface_tx` and `USB_interface_rx`).

Upstream traceability:
- [SYS_REQ_INTERFACE_1](../../sys/req/sys_interfaces.md#sys_req_interface_1)
- [SYS_ARCH_INTERFACE_1](../../sys/arch/sys_interfaces.md#sys_arch_interface_1)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 6.4, 8.3, and 33
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.12, 3.14, and 3.28

Status: approved

## Radio and RF interface

### HW_SW_INTERFACE_8

The 32 MHz external TCXO Y1 is connected to MCU `OSC_IN`; `OSC_OUT` is not connected.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (Y1 `ECS-TXO-25CSMV`, `OSC_IN` and `OSC_OUT`).

Upstream traceability:
- [SYS_REQ_PRODUCT_1](../../sys/req/sys_product.md#sys_req_product_1)
- [SYS_REQ_PARAMETER_11](../../sys/req/sys_parameters.md#sys_req_parameter_11)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_PARAMETER_11](../../sys/arch/sys_parameters.md#sys_arch_parameter_11)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 4.8.8 and 6.2.1
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.8.1, 3.12, and 5.3.9

Status: approved

### HW_SW_INTERFACE_9

The MCU-integrated sub-GHz radio is used to provide the system RF functionality.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (U1 `SUBGHZSPI`, NSS and radio control) and [RF front-end schematic](../../hw/KiCad/RF_front_end.kicad_sch) (U2 radio path).

Upstream traceability:
- [SYS_REQ_INTERFACE_2](../../sys/req/sys_interfaces.md#sys_req_interface_2)
- [SYS_REQ_PARAMETER_38](../../sys/req/sys_parameters.md#sys_req_parameter_38)
- [SYS_REQ_PARAMETER_39](../../sys/req/sys_parameters.md#sys_req_parameter_39)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_PARAMETER_38](../../sys/arch/sys_parameters.md#sys_arch_parameter_38)
- [SYS_ARCH_PARAMETER_39](../../sys/arch/sys_parameters.md#sys_arch_parameter_39)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapter 4.8
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapter 3.8

Status: approved

### HW_SW_INTERFACE_10

PA4 drives the `RF_SWITCH_CTRL` net to U2: low selects RX/idle and high selects TX.

Hardware design reference: [MCU schematic](../../hw/KiCad/MCU.kicad_sch) (`PA4`, `RF_SWITCH_CTRL`) and [RF front-end schematic](../../hw/KiCad/RF_front_end.kicad_sch) (U2 `SKY13453-385LF`, series resistor R4 to U2 `VTCL`, `RF_SWITCH_CTRL`, `RFO_HP`, `RFI_P`, `RFI_N`, `VR_PA`).

Upstream traceability:
- [SYS_REQ_INTERFACE_2](../../sys/req/sys_interfaces.md#sys_req_interface_2)
- [SYS_REQ_PARAMETER_35](../../sys/req/sys_parameters.md#sys_req_parameter_35)
- [SYS_REQ_PARAMETER_36](../../sys/req/sys_parameters.md#sys_req_parameter_36)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_PARAMETER_35](../../sys/arch/sys_parameters.md#sys_arch_parameter_35)
- [SYS_ARCH_PARAMETER_36](../../sys/arch/sys_parameters.md#sys_arch_parameter_36)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 4.3.2, 4.3.3, and 4.3.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.8.3 and 3.8.4

Status: approved

### HW_SW_INTERFACE_12

The RF front-end PA path uses the integrated radio `RFO_HP` connection. The `RFO_LP` path is not connected to the RF front-end.

Hardware design reference: [RF front-end schematic](../../hw/KiCad/RF_front_end.kicad_sch) (U2 `RFO_HP`, `VR_PA`, `RFI_P`, `RFI_N` and `RF_SWITCH_CTRL`) and [top-level schematic](../../hw/KiCad/top.kicad_sch) (U2 RF path).

Upstream traceability:
- [SYS_REQ_INTERFACE_2](../../sys/req/sys_interfaces.md#sys_req_interface_2)
- [SYS_REQ_PARAMETER_35](../../sys/req/sys_parameters.md#sys_req_parameter_35)
- [SYS_REQ_PARAMETER_36](../../sys/req/sys_parameters.md#sys_req_parameter_36)
- [SYS_ARCH_INTERFACE_2](../../sys/arch/sys_interfaces.md#sys_arch_interface_2)
- [SYS_ARCH_COMPONENT_1](../../sys/arch/sys_components.md#sys_arch_component_1)
- [SYS_ARCH_PARAMETER_35](../../sys/arch/sys_parameters.md#sys_arch_parameter_35)
- [SYS_ARCH_PARAMETER_36](../../sys/arch/sys_parameters.md#sys_arch_parameter_36)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1) Chapters 4.3.3 and 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2) Chapters 3.8.3 and 5.3.3

Status: approved
