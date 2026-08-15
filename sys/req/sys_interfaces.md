# System interfaces

# SYS_REQ_INTERFACE_1

Name: sysi_hostUsbControl

`STM32WLE5_RF_dongle` shall provide the following host USB control interface.

<a id="host-environment-and-scope"></a>
**Host environment and scope**
- A POSIX-compatible host shall be able to operate the interface from an interactive CLI and non-interactive scripts.

<a id="usb-transport"></a>
**USB transport**
- The host shall access the product through a bidirectional Virtual COM Port serial stream over USB 2.0 Full-Speed.
- The USB connection shall conform to USB 2.0 full-speed operation (12 Mbit/s).

<a id="framing-and-integrity"></a>
**Framing and integrity**
- All frames shall be COBS-encoded and terminated by an `0x00` byte delimiter.
- The decoded-frame fields shall be, in order:
  1. **Command ID:** 2 bytes, encoded least-significant byte first. It identifies the command.
  2. **Payload length:** 2 bytes, encoded least-significant byte first. It specifies the number of payload bytes and shall equal the actual number of payload bytes.
  3. **Payload:** the number of bytes specified by payload length. Its format and meaning shall be defined by the applicable command.
  4. **CRC:** 2 bytes, encoded least-significant byte first. It shall contain CRC-16/XMODEM calculated over the preceding decoded-frame fields: command ID, payload length, and payload.
- A frame that is invalid after COBS decoding, has an invalid format or fails CRC validation shall be discarded.

<a id="command-processing"></a>
**Command processing**
- Malformed or contextually invalid commands shall be discarded without a response and shall not affect an active RF operation.
- The system shall process one valid host command at a time. `SET_CONFIG`, `CALIBRATE`, `TX_PACKET`, `RX_START`, `CAD_START`, `TX_CONTINUOUS_WAVE`, `TX_CONTINUOUS_PREAMBLE`, and `STOP` shall terminate any active RF operation before performing their function. `GET_CONFIG_REQUEST` and `GET_DIAGNOSTICS_REQUEST` shall neither terminate nor modify an active RF operation.

<a id="host-to-target-requests"></a>
**Host-to-target requests**

| Command ID | Command name | Description | Payload length | Payload | Response |
|---:|---|---|---|---|---|
| <a id="get-config-request"></a>`0x0000` | `GET_CONFIG_REQUEST` | Request the current complete RF configuration. | 0 | - | [`GET_CONFIG_RESPONSE`](#get-config-response) |
| <a id="set-config-request"></a>`0x0001` | `SET_CONFIG` | Atomically apply a complete RF configuration for the selected packet type. | [Configuration Payload length](#configuration-payload) | [Configuration Payload](#configuration-payload) | - |
| <a id="calibrate-request"></a>`0x0002` | `CALIBRATE` | Perform complete radio calibration over the supplied image-calibration interval. | 2 | [Calibration Payload](#calibration-payload) | - |
| <a id="tx-packet-request"></a>`0x0003` | `TX_PACKET` | Start one packet transmission using the current configuration through [sysi_rfInputOutput](#sys_req_interface_2). | [TX Packet Payload length](#tx-packet-payload) | [TX Packet Payload](#tx-packet-payload) | - |
| <a id="rx-start-request"></a>`0x0004` | `RX_START` | Start the [configured RX operation](#rx-operation), receiving RF signals through [sysi_rfInputOutput](#sys_req_interface_2), when [sysp_packetType](sys_parameters.md#sys_req_parameter_1) is `lora` or `fsk`. | 0 | - | [`RX_PACKET`](#rx-packet) |
| <a id="cad-start-request"></a>`0x0005` | `CAD_START` | Start the [configured LoRa CAD operation](#cad-operation), detecting RF signals and, when applicable, receiving after detection through [sysi_rfInputOutput](#sys_req_interface_2). | 0 | - | [`CAD_DETECTED`](#cad-detected), and [`RX_PACKET`](#rx-packet) when applicable |
| <a id="tx-continuous-wave-request"></a>`0x0006` | `TX_CONTINUOUS_WAVE` | Start continuous-wave TX through [sysi_rfInputOutput](#sys_req_interface_2) using the current RF frequency and requested output power. | 0 | - | - |
| <a id="tx-continuous-preamble-request"></a>`0x0007` | `TX_CONTINUOUS_PREAMBLE` | Start infinite-preamble TX through [sysi_rfInputOutput](#sys_req_interface_2) using the current configuration when [sysp_packetType](sys_parameters.md#sys_req_parameter_1) is `lora`, `fsk`, or `msk`. | 0 | - | - |
| <a id="stop-request"></a>`0x0008` | `STOP` | Terminate any active RF operation. | 0 | - | - |
| <a id="get-diagnostics-request"></a>`0x0009` | `GET_DIAGNOSTICS_REQUEST` | Retrieve current functional RF operation and radio errors. | 0 | - | [`GET_DIAGNOSTICS_RESPONSE`](#get-diagnostics-response) |

<a id="target-to-host-responses"></a>
**Target-to-host responses**

| Command ID | Command name | Description | Payload length | Payload | Request |
|---:|---|---|---|---|---|
| <a id="get-config-response"></a>`0x8001` | `GET_CONFIG_RESPONSE` | Return the current complete RF configuration. | [Configuration Payload length](#configuration-payload) | [Configuration Payload](#configuration-payload) | [Host request](#get-config-request) |
| <a id="get-diagnostics-response"></a>`0x8002` | `GET_DIAGNOSTICS_RESPONSE` | Return current functional RF operation and radio errors. | 3 | [Diagnostics Payload](#diagnostics-payload) | [Host request](#get-diagnostics-request) |
| <a id="rx-packet"></a>`0x8003` | `RX_PACKET` | Report one received packet during active `RX_START` or during the receive phase of active `CAD_START`. | [RX Packet Payload length](#rx-packet-payload) | [RX Packet Payload](#rx-packet-payload) | Active `RX_START`, or active `CAD_START` with CAD exit mode `receive_on_detection` |
| <a id="cad-detected"></a>`0x8004` | `CAD_DETECTED` | Report detected channel activity during active `CAD_START`. | 0 | - | Active [`CAD_START`](#cad-start-request) |

<a id="configuration-payload"></a>
**Configuration payload**

The Configuration Payload shall consist in the same order, of one common fixed prefix followed by exactly one fixed packet-type block. The first field, [sysp_packetType](sys_parameters.md#sys_req_parameter_1), shall select the packet-type field block.

Common prefix:
1. [sysp_packetType](sys_parameters.md#sys_req_parameter_1)
2. [sysp_rfFrequencyHz](sys_parameters.md#sys_req_parameter_2)
3. [sysp_payloadLengthBytes](sys_parameters.md#sys_req_parameter_3)
4. [sysp_txOutputPowerDbm](sys_parameters.md#sys_req_parameter_35)

`lora` block:
1. [sysp_loraSpreadingFactor](sys_parameters.md#sys_req_parameter_4)
2. [sysp_loraBandwidthKHz](sys_parameters.md#sys_req_parameter_5)
3. [sysp_loraCodingRate](sys_parameters.md#sys_req_parameter_6)
4. [sysp_loraLowDataRateOptimize](sys_parameters.md#sys_req_parameter_7)
5. [sysp_preambleLength](sys_parameters.md#sys_req_parameter_13)
6. [sysp_loraHeaderMode](sys_parameters.md#sys_req_parameter_14)
7. [sysp_loraPayloadCrcEnabled](sys_parameters.md#sys_req_parameter_15)
8. [sysp_loraInvertIq](sys_parameters.md#sys_req_parameter_16)
9. [sysp_loraSynchronizationWord](sys_parameters.md#sys_req_parameter_17)
10. [sysp_operationTimeout](sys_parameters.md#sys_req_parameter_37)
11. [sysp_loraSymbolTimeout](sys_parameters.md#sys_req_parameter_33)
12. [sysp_txRampTimeUs](sys_parameters.md#sys_req_parameter_36)
13. [sysp_cad](sys_parameters.md#sys_req_parameter_34)
14. [sysp_rxMode](sys_parameters.md#sys_req_parameter_30)
15. [sysp_rxTimeoutControl](sys_parameters.md#sys_req_parameter_31)
16. [sysp_rxDutyCycle](sys_parameters.md#sys_req_parameter_32)

`fsk` block:
1. [sysp_genericBitRateBps](sys_parameters.md#sys_req_parameter_8)
2. [sysp_genericFrequencyDeviationHz](sys_parameters.md#sys_req_parameter_9)
3. [sysp_genericPulseShape](sys_parameters.md#sys_req_parameter_10)
4. [sysp_fskReceiveBandwidthKHz](sys_parameters.md#sys_req_parameter_11)
5. [sysp_preambleLength](sys_parameters.md#sys_req_parameter_13)
6. [sysp_operationTimeout](sys_parameters.md#sys_req_parameter_37)
7. [sysp_genericPayloadLengthMode](sys_parameters.md#sys_req_parameter_18)
8. [sysp_genericPreambleDetectionLengthBits](sys_parameters.md#sys_req_parameter_19)
9. [sysp_genericSyncWordLengthBits](sys_parameters.md#sys_req_parameter_20)
10. [sysp_genericSyncWord](sys_parameters.md#sys_req_parameter_21)
11. [sysp_genericAddressFiltering](sys_parameters.md#sys_req_parameter_22)
12. [sysp_genericNodeAddress](sys_parameters.md#sys_req_parameter_23)
13. [sysp_genericBroadcastAddress](sys_parameters.md#sys_req_parameter_24)
14. [sysp_genericCrcType](sys_parameters.md#sys_req_parameter_25)
15. [sysp_genericCrcInitialValue](sys_parameters.md#sys_req_parameter_26)
16. [sysp_genericCrcPolynomial](sys_parameters.md#sys_req_parameter_27)
17. [sysp_genericWhiteningEnabled](sys_parameters.md#sys_req_parameter_28)
18. [sysp_genericWhiteningInitialValue](sys_parameters.md#sys_req_parameter_29)
19. [sysp_txRampTimeUs](sys_parameters.md#sys_req_parameter_36)
20. [sysp_rxMode](sys_parameters.md#sys_req_parameter_30)
21. [sysp_rxTimeoutControl](sys_parameters.md#sys_req_parameter_31)
22. [sysp_rxDutyCycle](sys_parameters.md#sys_req_parameter_32)

`msk` block:
1. [sysp_genericBitRateBps](sys_parameters.md#sys_req_parameter_8)
2. [sysp_genericPulseShape](sys_parameters.md#sys_req_parameter_10)
3. [sysp_preambleLength](sys_parameters.md#sys_req_parameter_13)
4. [sysp_operationTimeout](sys_parameters.md#sys_req_parameter_37)
5. [sysp_genericPayloadLengthMode](sys_parameters.md#sys_req_parameter_18)
6. [sysp_genericSyncWordLengthBits](sys_parameters.md#sys_req_parameter_20)
7. [sysp_genericSyncWord](sys_parameters.md#sys_req_parameter_21)
8. [sysp_genericAddressFiltering](sys_parameters.md#sys_req_parameter_22)
9. [sysp_genericNodeAddress](sys_parameters.md#sys_req_parameter_23)
10. [sysp_genericBroadcastAddress](sys_parameters.md#sys_req_parameter_24)
11. [sysp_genericCrcType](sys_parameters.md#sys_req_parameter_25)
12. [sysp_genericCrcInitialValue](sys_parameters.md#sys_req_parameter_26)
13. [sysp_genericCrcPolynomial](sys_parameters.md#sys_req_parameter_27)
14. [sysp_genericWhiteningEnabled](sys_parameters.md#sys_req_parameter_28)
15. [sysp_genericWhiteningInitialValue](sys_parameters.md#sys_req_parameter_29)
16. [sysp_txRampTimeUs](sys_parameters.md#sys_req_parameter_36)

`bpsk` block:
1. [sysp_bpskBitRateBps](sys_parameters.md#sys_req_parameter_12)
2. [sysp_operationTimeout](sys_parameters.md#sys_req_parameter_37)

> **Note**: All multi-byte parameter values shall be encoded least-significant byte first.
>

> **Note**: Payload Length shall equal the exact serialized byte length of the common fixed prefix and the one field block selected by [sysp_packetType](sys_parameters.md#sys_req_parameter_1).
>

<a id="rx-operation"></a>
**RX operation**

- In `single` mode, the operation shall end after the first [`RX_PACKET`](#rx-packet) or expiration of the applicable [sysp_rxTimeoutControl](sys_parameters.md#sys_req_parameter_31).
- In `continuous` and `duty_cycle` modes, the operation shall report each [`RX_PACKET`](#rx-packet) and remain active after each report.

<a id="cad-operation"></a>
**CAD operation**

- [`CAD_START`](#cad-start-request) shall repeat scans while no activity is detected.
- On detection, the target shall report [`CAD_DETECTED`](#cad-detected). With `stop_on_detection`, the operation shall end. With `receive_on_detection`, reception shall continue until one [`RX_PACKET`](#rx-packet) is reported or the configured [CAD timeout](sys_parameters.md#sys_req_parameter_34) expires, after which scanning shall resume.

<a id="calibration-payload"></a>
**Calibration Payload**

The Calibration Payload shall contain, in order:
1. [sysp_imageCalibrationLowerFrequencyHz](sys_parameters.md#sys_req_parameter_38)
2. [sysp_imageCalibrationUpperFrequencyHz](sys_parameters.md#sys_req_parameter_39).

> **Note**: All multi-byte Calibration Payload parameter values shall be encoded least-significant byte first.
>

<a id="tx-packet-payload"></a>
**TX Packet Payload**

The TX Packet Payload shall consist of [sysp_txPacketData](sys_parameters.md#sys_req_parameter_40).

<a id="rx-packet-payload"></a>
**RX Packet Payload**

The RX Packet Payload shall contain, in order:
1. [sysp_receivedPacketLengthBytes](sys_parameters.md#sys_req_parameter_41)
2. [sysp_rxPacketData](sys_parameters.md#sys_req_parameter_42)
3. [sysp_receivedPacketRssiDbm](sys_parameters.md#sys_req_parameter_43)
4. [sysp_receivedPacketSnrDb](sys_parameters.md#sys_req_parameter_44) for `lora`, or [sysp_receivedPacketStatus](sys_parameters.md#sys_req_parameter_45) for `fsk`.

> **Note**: Payload length shall equal the serialized size of [sysp_rxPacketData](sys_parameters.md#sys_req_parameter_42) plus 3 bytes for [sysp_receivedPacketLengthBytes](sys_parameters.md#sys_req_parameter_41), [sysp_receivedPacketRssiDbm](sys_parameters.md#sys_req_parameter_43), and either [sysp_receivedPacketSnrDb](sys_parameters.md#sys_req_parameter_44) or [sysp_receivedPacketStatus](sys_parameters.md#sys_req_parameter_45).

<a id="diagnostics-payload"></a>
**Diagnostics Payload**

The Diagnostics Payload shall contain, in order:
1. [sysp_activeRfOperation](sys_parameters.md#sys_req_parameter_46)
2. [sysp_radioErrors](sys_parameters.md#sys_req_parameter_47).

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_2](../../spec/specs.md#spec_2)
- [SPEC_3](../../spec/specs.md#spec_3)
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)

Status: approved

# SYS_REQ_INTERFACE_2

Name: sysi_rfInputOutput

`STM32WLE5_RF_dongle` shall provide one external bidirectional, half-duplex physical sub-GHz RF port shared for transmit and receive operation over the frequency range defined by [sysp_rfFrequencyHz](sys_parameters.md#sys_req_parameter_2).

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.2, Chapter 4.3.2, Chapter 4.3.3, Chapter 4.3.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1, Chapter 3.8.3, Chapter 3.8.4

Status: approved
