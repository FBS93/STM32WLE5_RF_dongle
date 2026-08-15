# RF semantic parameter requirements

Source conventions: RM0461 Rev. 11 §4.5–4.8, pp. 112–154; DS13105 Rev. 12 §3.8 and §5.3.3, pp. 18–22 and 69–75.

Encoding convention: Physical value = encoded value × Factor + Offset.

`Encoded value type` defines how the serialized field is interpreted.

# SYS_REQ_PARAMETER_1

Name: sysp_packetType

Defines RF packet type.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `fsk` (`0x00`)
- `lora` (`0x01`)
- `bpsk` (`0x02`)
- `msk` (`0x03`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1

Status: approved

# SYS_REQ_PARAMETER_2

Name: sysp_rfFrequencyHz

Defines RF operating frequency.

Value size: 4 bytes.

Encoded value type: unsigned integer.

Encoding:
- Factor: (32000000 / 2^25) Hz.
- Offset: 0 Hz.

Encoded range: [0x09600000, 0x3C000000].

Physical range: [150000000, 960000000] Hz.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.3.5, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.5

Status: approved

# SYS_REQ_PARAMETER_3

Name: sysp_payloadLengthBytes

Defines LoRa, generic FSK/GFSK, generic MSK/GMSK, or BPSK packet length.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1 byte.
- Offset: 0 bytes.

Encoded range: [0x00, 0xFF].

Physical range: [0, 255] bytes.

Dependencies and constraints:
- For generic framing with [sysp_genericAddressFiltering](#sys_req_parameter_22) set to `node` or `node_and_broadcast`, [sysp_payloadLengthBytes](#sys_req_parameter_3) ≤ 254 bytes.
- For BPSK, the packet length shall include the complete supplied frame: preamble, synchronization word, device ID, and CRC.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.2, Chapter 4.5.5, Chapter 4.5.7, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_4

Name: sysp_loraSpreadingFactor

Defines LoRa spreading factor.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- 5 (`0x05`)
- 6 (`0x06`)
- 7 (`0x07`)
- 8 (`0x08`)
- 9 (`0x09`)
- 10 (`0x0A`)
- 11 (`0x0B`)
- 12 (`0x0C`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.1, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1

Status: approved

# SYS_REQ_PARAMETER_5

Name: sysp_loraBandwidthKHz

Defines LoRa bandwidth.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- 7.81 kHz (`0x00`)
- 10.42 kHz (`0x08`)
- 15.63 kHz (`0x01`)
- 20.83 kHz (`0x09`)
- 31.25 kHz (`0x02`)
- 41.67 kHz (`0x0A`)
- 62.5 kHz (`0x03`)
- 125 kHz (`0x04`)
- 250 kHz (`0x05`)
- 500 kHz (`0x06`)

Dependencies and constraints:
- When [sysp_rfFrequencyHz](#sys_req_parameter_2) < 400 MHz, 250 and 500 kHz are not supported.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.1, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1, Chapter 5.3.3

Status: approved

# SYS_REQ_PARAMETER_6

Name: sysp_loraCodingRate

Defines LoRa coding rate.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `4/4` (`0x00`)
- `4/5` (`0x01`)
- `4/6` (`0x02`)
- `4/7` (`0x03`)
- `4/8` (`0x04`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.1, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1

Status: approved

# SYS_REQ_PARAMETER_7

Name: sysp_loraLowDataRateOptimize

Defines whether LoRa low-data-rate optimization is enabled.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `disabled` (`0x00`)
- `enabled` (`0x01`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.1, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1

Status: approved

# SYS_REQ_PARAMETER_8

Name: sysp_genericBitRateBps

Defines generic FSK/GFSK RX/TX or MSK/GMSK TX bit rate.

Value size: 3 bytes.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1 bit/s.
- Offset: 0 bit/s.

Encoded range:
- FSK/GFSK: [0x000258, 0x0493E0].
- MSK/GMSK: [0x000064, 0x002710].

Physical range:
- FSK/GFSK: [600, 300000] bit/s.
- MSK/GMSK: [100, 10000] bit/s.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.1, Chapter 4.5.3, Chapter 4.5.4, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1, Chapter 5.3.3

Status: approved

# SYS_REQ_PARAMETER_9

Name: sysp_genericFrequencyDeviationHz

Defines FSK/GFSK frequency deviation.

Value size: 3 bytes.

Encoded value type: unsigned integer.

Encoding:
- Factor: (32000000 / 2^25) Hz.
- Offset: 0 Hz.

Encoded range: [0x000276, 0x033333].

Physical range: [600.8148193359375, 199999.80926513671875] Hz.

Dependencies and constraints:
- [sysp_genericFrequencyDeviationHz](#sys_req_parameter_9) + ([sysp_genericBitRateBps](#sys_req_parameter_8) / 2) ≤ 250000 Hz.
- When [sysp_fskReceiveBandwidthKHz](#sys_req_parameter_11) applies, [sysp_genericFrequencyDeviationHz](#sys_req_parameter_9) + ([sysp_genericBitRateBps](#sys_req_parameter_8) / 2) < [sysp_fskReceiveBandwidthKHz](#sys_req_parameter_11) expressed in Hz.
- For GMSK configured through [sysp_packetType](#sys_req_parameter_1) set to `fsk`, [sysp_genericFrequencyDeviationHz](#sys_req_parameter_9) = ([sysp_genericBitRateBps](#sys_req_parameter_8) / 4).

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.3, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 5.3.3

Status: approved

# SYS_REQ_PARAMETER_10

Name: sysp_genericPulseShape

Defines generic FSK/GFSK or MSK/GMSK TX pulse shape.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `none` (`0x00`)
- `gaussian_bt_0_3` (`0x08`)
- `gaussian_bt_0_5` (`0x09`)
- `gaussian_bt_0_7` (`0x0A`)
- `gaussian_bt_1_0` (`0x0B`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.3, Chapter 4.5.4, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.3

Status: approved

# SYS_REQ_PARAMETER_11

Name: sysp_fskReceiveBandwidthKHz

Defines FSK/GFSK RX bandwidth.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- 4.8 kHz (`0x1F`)
- 5.8 kHz (`0x17`)
- 7.3 kHz (`0x0F`)
- 9.7 kHz (`0x1E`)
- 11.7 kHz (`0x16`)
- 14.6 kHz (`0x0E`)
- 19.5 kHz (`0x1D`)
- 23.4 kHz (`0x15`)
- 29.3 kHz (`0x0D`)
- 39.0 kHz (`0x1C`)
- 46.9 kHz (`0x14`)
- 58.6 kHz (`0x0C`)
- 78.2 kHz (`0x1B`)
- 93.8 kHz (`0x13`)
- 117.3 kHz (`0x0B`)
- 156.2 kHz (`0x1A`)
- 187.2 kHz (`0x12`)
- 234.3 kHz (`0x0A`)
- 312.0 kHz (`0x19`)
- 373.6 kHz (`0x11`)
- 467.0 kHz (`0x09`)

Dependencies and constraints:
- [sysp_fskReceiveBandwidthKHz](#sys_req_parameter_11) expressed in Hz > ([sysp_genericBitRateBps](#sys_req_parameter_8) + (2 × [sysp_genericFrequencyDeviationHz](#sys_req_parameter_9)) + the frequency-error term).
- The frequency-error term = (2 × the HSE32 frequency error), with both expressed in Hz.
- The HSE32 frequency error is the maximum absolute deviation, expressed in Hz, of the implemented HSE32 frequency from 32000000 Hz over the applicable operating conditions.

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.3, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1

Status: approved

# SYS_REQ_PARAMETER_12

Name: sysp_bpskBitRateBps

Defines BPSK TX bit rate.

Value size: 3 bytes.

Value type: enumeration.

Allowed values:
- 100 bit/s (`0x9C4000`)
- 600 bit/s (`0x1A0AAA`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.6, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.1

Status: approved

# SYS_REQ_PARAMETER_13

Name: sysp_preambleLength

Defines LoRa or generic FSK/GFSK or MSK/GMSK packet preamble length.

Value size: 2 bytes.

Encoded value type: unsigned integer.

Encoding:
- LoRa:
  - Factor: 1 symbol.
  - Offset: 0 symbols.
- Generic FSK/GFSK and MSK/GMSK:
  - Factor: 1 bit.
  - Offset: 0 bits.

Encoded range:
- LoRa: [0x0001, 0xFFFF].
- Generic FSK/GFSK: [0x0008, 0xFFFF].
- Generic MSK/GMSK: [0x0008, 0xFFFF].

Physical range:
- LoRa: [1, 65535] symbols.
- Generic FSK/GFSK: [8, 65535] bits.
- Generic MSK/GMSK: [8, 65535] bits.

Dependencies and constraints:
- For LoRa [sysp_loraSpreadingFactor](#sys_req_parameter_4) values 5 and 6, [sysp_preambleLength](#sys_req_parameter_13) ≥ 12 symbols.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.1, Chapter 4.5.2, Chapter 4.5.5, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_14

Name: sysp_loraHeaderMode

Defines LoRa header mode.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `explicit` (`0x00`)
- `implicit` (`0x01`)

Dependencies and constraints:
- `explicit` shall use a variable packet length.
- `implicit` shall use a fixed packet length without a header, and [sysp_payloadLengthBytes](#sys_req_parameter_3), [sysp_loraCodingRate](#sys_req_parameter_6), and [sysp_loraPayloadCrcEnabled](#sys_req_parameter_15) shall agree at both ends of the radio link.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.2, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_15

Name: sysp_loraPayloadCrcEnabled

Defines whether LoRa packet CRC is enabled.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `disabled` (`0x00`)
- `enabled` (`0x01`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.2, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_16

Name: sysp_loraInvertIq

Defines whether LoRa IQ setting is standard or inverted.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `standard` (`0x00`)
- `inverted` (`0x01`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_17

Name: sysp_loraSynchronizationWord

Defines the LoRa synchronization word and network type.

Value size: 2 bytes.

Value type: enumeration.

Allowed values:
- `private` (`0x1424`)
- `public` (`0x3444`)

All remaining values are reserved.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.10.33, Chapter 4.10.34

Status: approved

# SYS_REQ_PARAMETER_18

Name: sysp_genericPayloadLengthMode

Defines generic FSK/GFSK or MSK/GMSK packet-length mode.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `fixed` (`0x00`)
- `variable` (`0x01`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_19

Name: sysp_genericPreambleDetectionLengthBits

Defines generic FSK/GFSK RX preamble-detection length.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `disabled` (`0x00`)
- 8 bits (`0x04`)
- 16 bits (`0x05`)
- 24 bits (`0x06`)
- 32 bits (`0x07`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_20

Name: sysp_genericSyncWordLengthBits

Defines generic FSK/GFSK or MSK/GMSK synchronization-word length.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1 bit.
- Offset: 0 bits.

Encoded range: [0x00, 0x40].

Physical range: [0, 64] bits.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_21

Name: sysp_genericSyncWord

Defines the 8-byte generic FSK/GFSK or MSK/GMSK synchronization word.

Value size: 8 bytes.

Value type: byte array.

Dependencies and constraints:
- The number of synchronization-word bits used shall equal [sysp_genericSyncWordLengthBits](#sys_req_parameter_20).

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4, Chapter 4.10.18–4.10.25

Status: approved

# SYS_REQ_PARAMETER_22

Name: sysp_genericAddressFiltering

Defines generic FSK/GFSK or MSK/GMSK address-filtering mode.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `disabled` (`0x00`)
- `node` (`0x01`)
- `node_and_broadcast` (`0x02`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_23

Name: sysp_genericNodeAddress

Defines the generic FSK/GFSK or MSK/GMSK node address.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1.
- Offset: 0.

Encoded range: [0x00, 0xFF].

Physical range: [0, 255].

Dependencies and constraints:
- This parameter applies only when [sysp_genericAddressFiltering](#sys_req_parameter_22) is `node` or `node_and_broadcast`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4, Chapter 4.10.26

Status: approved

# SYS_REQ_PARAMETER_24

Name: sysp_genericBroadcastAddress

Defines the generic FSK/GFSK or MSK/GMSK broadcast address.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1.
- Offset: 0.

Encoded range: [0x00, 0xFF].

Physical range: [0, 255].

Dependencies and constraints:
- This parameter applies only when [sysp_genericAddressFiltering](#sys_req_parameter_22) is `node_and_broadcast`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4, Chapter 4.10.27

Status: approved

# SYS_REQ_PARAMETER_25

Name: sysp_genericCrcType

Defines generic FSK/GFSK or MSK/GMSK CRC type.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `crc8` (`0x00`)
- `none` (`0x01`)
- `crc16` (`0x02`)
- `crc8_inverted` (`0x04`)
- `crc16_inverted` (`0x06`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_26

Name: sysp_genericCrcInitialValue

Defines generic FSK/GFSK or MSK/GMSK CRC initial value.

Value size: 2 bytes.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1.
- Offset: 0.

Encoded range: [0x0000, 0xFFFF].

Physical range: [0, 65535].

Dependencies and constraints:
- This parameter applies only when [sysp_genericCrcType](#sys_req_parameter_25) is not `none`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4, Chapter 4.10.14–4.10.15

Status: approved

# SYS_REQ_PARAMETER_27

Name: sysp_genericCrcPolynomial

Defines generic FSK/GFSK or MSK/GMSK CRC polynomial.

Value size: 2 bytes.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1.
- Offset: 0.

Encoded range: [0x0000, 0xFFFF].

Physical range: [0, 65535].

Dependencies and constraints:
- This parameter applies only when [sysp_genericCrcType](#sys_req_parameter_25) is not `none`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4, Chapter 4.10.16–4.10.17

Status: approved

# SYS_REQ_PARAMETER_28

Name: sysp_genericWhiteningEnabled

Defines whether generic FSK/GFSK or MSK/GMSK whitening is enabled.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `disabled` (`0x00`)
- `enabled` (`0x01`)

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_29

Name: sysp_genericWhiteningInitialValue

Defines generic FSK/GFSK or MSK/GMSK whitening initial value.

Value size: 2 bytes.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1.
- Offset: 0.

Encoded range: [0x0000, 0x01FF].

Physical range: [0, 511].

Dependencies and constraints:
- This parameter applies only when [sysp_genericWhiteningEnabled](#sys_req_parameter_28) is `enabled`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.5, Chapter 4.8.4, Chapter 4.10.11–4.10.12

Status: approved

# SYS_REQ_PARAMETER_30

Name: sysp_rxMode

Selects functional LoRa or generic FSK/GFSK receive behavior.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `single` (`0x00`)
- `continuous` (`0x01`)
- `duty_cycle` (`0x02`)

All remaining values are reserved.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.3

Status: approved

# SYS_REQ_PARAMETER_31

Name: sysp_rxTimeoutControl

Defines the timeout of a `single` LoRa or generic FSK/GFSK RX operation.

Value size: 3 bytes.

Encoded value type: unsigned integer with named values.

Allowed named values:
- `no_timeout` (`0x000000`)

Encoding:
- Finite timeouts:
  - Factor: 15.625 µs.
  - Offset: 0 µs.

Encoded range:
- Finite timeouts: [0x000001, 0xFFFFFE].

Physical range:
- Finite timeouts: [0.000015625, 262.14396875] s. `no_timeout` is outside the finite-timeout domain.

Dependencies and constraints:
- This parameter applies only when [sysp_rxMode](#sys_req_parameter_30) is `single`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.3

Status: approved

# SYS_REQ_PARAMETER_32

Name: sysp_rxDutyCycle

Defines LoRa or generic FSK/GFSK RX duty-cycle listening periods.

Value size: 6 bytes.

Value type: structure.

RX period:
- Value size: 3 bytes.
- Encoded value type: unsigned integer with named values.
- Allowed named values:
  - `until_packet` (`0x000000`)
- Encoding:
  - Finite periods:
    - Factor: 15.625 µs.
    - Offset: 0 µs.
- Encoded range:
  - Finite periods: [0x000001, 0xFFFFFF].
- Physical range:
  - Finite periods: [0.000015625, 262.143984375] s. The named value `until_packet` is outside the finite-period domain.

Sleep period:
- Value size: 3 bytes.
- Encoded value type: unsigned integer.
- Encoding:
  - Factor: 15.625 µs.
  - Offset: 0 µs.
- Encoded range: [0x000000, 0xFFFFFF].
- Physical range: [0, 262.143984375] s.

Dependencies and constraints:
- This parameter applies only when [sysp_rxMode](#sys_req_parameter_30) is `duty_cycle`.
- For generic FSK/GFSK, (preamble duration + synchronization-word duration) < ((2 × RX period) + sleep period).
- For LoRa, (preamble duration + header duration) < ((2 × RX period) + sleep period).

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.3

Status: approved

# SYS_REQ_PARAMETER_33

Name: sysp_loraSymbolTimeout

Defines LoRa RX end-of-preamble detection timeout.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1 symbol.
- Offset: 0 symbols.

Encoded range: [0x00, 0xFF].

Physical range: [0, 255] symbols.

Dependencies and constraints:
- 0 shall disable the end-of-preamble detection timeout.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_34

Name: sysp_cad

Defines LoRa channel-activity-detection settings.

Value size: 7 bytes.

Value type: structure.

Scan-symbol count:
- Value size: 1 byte.
- Value type: enumeration.
- Allowed values:
  - 1 symbol (`0x00`)
  - 2 symbols (`0x01`)
  - 4 symbols (`0x02`)
  - 8 symbols (`0x03`)
  - 16 symbols (`0x04`)

Detection peak threshold:
- Value size: 1 byte.
- Encoded value type: unsigned integer.
- Encoding:
  - Factor: 1.
  - Offset: 0.
- Encoded range: [0x00, 0xFF].
- Physical range: [0, 255].

Detection minimum threshold:
- Value size: 1 byte.
- Encoded value type: unsigned integer.
- Encoding:
  - Factor: 1.
  - Offset: 0.
- Encoded range: [0x00, 0xFF].
- Physical range: [0, 255].

Exit mode:
- Value size: 1 byte.
- Value type: enumeration.
- Allowed values:
  - `stop_on_detection` (`0x00`)
  - `receive_on_detection` (`0x01`)

Timeout:
- Value size: 3 bytes.
- Encoded value type: unsigned integer.
- Encoding:
  - Factor: 15.625 µs.
  - Offset: 0 µs.
- Encoded range: [0x000000, 0xFFFFFF].
- Physical range: [0, 262.143984375] s.

Dependencies and constraints:
- The timeout applies only when exit mode is `receive_on_detection`.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.2, Chapter 4.8.3, Chapter 4.8.4

Status: approved

# SYS_REQ_PARAMETER_35

Name: sysp_txOutputPowerDbm

Defines requested TX output-power setting.

Value size: 1 byte.

Encoded value type: signed integer (two's-complement byte encoding).

Encoding:
- Factor: 1 dBm.
- Offset: 0 dBm.

Encoded range: [0xEF, 0xFF] and [0x00, 0x16] (signed values [-17, -1] and [0, 22]).

Physical range: [-17, 22] dBm.

Dependencies and constraints:
- The software shall derive the HP/LP PA path and safe PA settings from this requested power, [sysp_rfFrequencyHz](#sys_req_parameter_2), and the implemented RF hardware.

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.3.3, Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.3, Chapter 5.3.3

Status: approved

# SYS_REQ_PARAMETER_36

Name: sysp_txRampTimeUs

Defines FSK/GFSK, MSK/GMSK or LoRa TX PA ramp-up time.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- 10 µs (`0x00`)
- 20 µs (`0x01`)
- 40 µs (`0x02`)
- 80 µs (`0x03`)
- 200 µs (`0x04`)
- 800 µs (`0x05`)
- 1700 µs (`0x06`)
- 3400 µs (`0x07`)

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.4
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.3

Status: approved

# SYS_REQ_PARAMETER_37

Name: sysp_operationTimeout

Defines packet TX operation timeout.

Value size: 3 bytes.

Encoded value type: unsigned integer with named values.

Allowed named values:
- `disabled` (`0x000000`)

Encoding:
- Finite timeouts:
  - Factor: 15.625 µs.
  - Offset: 0 µs.

Encoded range:
- Finite timeouts: [0x000001, 0xFFFFFF].

Physical range:
- Finite timeouts: [0.000015625, 262.143984375] s. The named value `disabled` is outside the finite-timeout domain.

Domain:
- sw

Upstream traceability:
- [SPEC_5](../../spec/specs.md#spec_5)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.3

Status: approved

# SYS_REQ_PARAMETER_38

Name: sysp_imageCalibrationLowerFrequencyHz

Defines lower image-calibration frequency.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 4000000 Hz.
- Offset: 0 Hz.

Encoded range: [0x26, 0xF0].

Physical range: [152000000, 960000000] Hz.

Dependencies and constraints:
- [sysp_imageCalibrationLowerFrequencyHz](#sys_req_parameter_38) ≤ [sysp_imageCalibrationUpperFrequencyHz](#sys_req_parameter_39).

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.7
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.4, Chapter 3.8.5

Status: approved

# SYS_REQ_PARAMETER_39

Name: sysp_imageCalibrationUpperFrequencyHz

Defines upper image-calibration frequency.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 4000000 Hz.
- Offset: 0 Hz.

Encoded range: [0x26, 0xF0].

Physical range: [152000000, 960000000] Hz.

Dependencies and constraints:
- [sysp_imageCalibrationUpperFrequencyHz](#sys_req_parameter_39) ≥ [sysp_imageCalibrationLowerFrequencyHz](#sys_req_parameter_38).

Domain:
- sw
- hw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.7
- [EHW_DOC_2](../../spec/external_hw_docs.md#ehw_doc_2), Chapter 3.8.4, Chapter 3.8.5

Status: approved

# SYS_REQ_PARAMETER_40

Name: sysp_txPacketData

Defines TX packet content.

Value size: [0, 255] bytes.

Value type: byte array.

Dependencies and constraints:
- The packet data length shall equal [sysp_payloadLengthBytes](#sys_req_parameter_3).
- For BPSK, [sysp_txPacketData](#sys_req_parameter_40) shall contain the complete frame, including preamble, synchronization word, device ID, and CRC.

Domain:
- sw

Upstream traceability:
- [SPEC_5](../../spec/specs.md#spec_5)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.5.7, Chapter 4.6, Chapter 4.8.2

Status: approved

# SYS_REQ_PARAMETER_41

Name: sysp_receivedPacketLengthBytes

Reports RX packet length.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: 1 byte.
- Offset: 0 bytes.

Encoded range: [0x00, 0xFF].

Physical range: [0, 255] bytes.

Dependencies and constraints:
- [sysp_receivedPacketLengthBytes](#sys_req_parameter_41) shall equal the length of [sysp_rxPacketData](#sys_req_parameter_42).

Domain:
- sw

Upstream traceability:
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.5

Status: approved

# SYS_REQ_PARAMETER_42

Name: sysp_rxPacketData

Reports packet content for received packets.

Value size: [0, 255] bytes.

Value type: byte array.

Dependencies and constraints:
- The packet data length shall equal [sysp_receivedPacketLengthBytes](#sys_req_parameter_41).

Domain:
- sw

Upstream traceability:
- [SPEC_5](../../spec/specs.md#spec_5)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.6, Chapter 4.8.5

Status: approved

# SYS_REQ_PARAMETER_43

Name: sysp_receivedPacketRssiDbm

Reports average RX packet RSSI for LoRa or FSK/GFSK.

Value size: 1 byte.

Encoded value type: unsigned integer.

Encoding:
- Factor: -0.5 dBm.
- Offset: 0 dBm.

Encoded range: [0x00, 0xFF].

Physical range: [-127.5, 0] dBm.

Domain:
- sw

Upstream traceability:
- [SPEC_6](../../spec/specs.md#spec_6)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.5

Status: approved

# SYS_REQ_PARAMETER_44

Name: sysp_receivedPacketSnrDb

Reports RX packet SNR exclusively for LoRa.

Value size: 1 byte.

Encoded value type: signed integer (two's-complement byte encoding).

Encoding:
- Factor: 0.25 dB.
- Offset: 0 dB.

Encoded range: [0x80, 0xFF] and [0x00, 0x7F] (signed values [-128, -1] and [0, 127]).

Physical range: [-32, 31.75] dB.

Domain:
- sw

Upstream traceability:
- [SPEC_6](../../spec/specs.md#spec_6)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.5

Status: approved

# SYS_REQ_PARAMETER_45

Name: sysp_receivedPacketStatus

Reports RX packet status exclusively for FSK/GFSK.

Value size: 1 byte.

Value type: bit field.

Allowed flags:
- `preamble_error` (`0x80`)
- `sync_error` (`0x40`)
- `address_error` (`0x20`)
- `crc_error` (`0x10`)
- `length_error` (`0x08`)
- `abort_error` (`0x04`)
- `packet_received` (`0x02`)
- `packet_sent` (`0x01`)

Dependencies and constraints:
- Allowed flags may be combined by bitwise OR.

Domain:
- sw

Upstream traceability:
- [SPEC_6](../../spec/specs.md#spec_6)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.5

Status: approved

# SYS_REQ_PARAMETER_46

Name: sysp_activeRfOperation

Reports the current host-visible RF operation without exposing MCU radio states.

Value size: 1 byte.

Value type: enumeration.

Allowed values:
- `idle` (`0x00`)
- `packet_tx` (`0x01`)
- `rx` (`0x02`)
- `cad` (`0x03`)
- `continuous_wave_tx` (`0x04`)
- `continuous_preamble_tx` (`0x05`)
- `calibration` (`0x06`)

Domain:
- sw

Upstream traceability:
- [SPEC_6](../../spec/specs.md#spec_6)

Status: approved
# SYS_REQ_PARAMETER_47

Name: sysp_radioErrors

Reports recorded radio operational errors.

Value size: 2 bytes.

Value type: bit field.

Allowed flags:
- `rc64k_calibration_error` (`0x0001`)
- `rc13m_calibration_error` (`0x0002`)
- `pll_calibration_error` (`0x0004`)
- `adc_calibration_error` (`0x0008`)
- `image_calibration_error` (`0x0010`)
- `hse32_start_error` (`0x0020`)
- `pll_lock_error` (`0x0040`)
- `pa_ramp_error` (`0x0100`)

Dependencies and constraints:
- Zero shall indicate that no radio operational error is recorded.
- Allowed flags may be combined by bitwise OR.
- Bits 15:9 and bit 7 shall be zero.

Domain:
- sw

Upstream traceability:
- [SPEC_6](../../spec/specs.md#spec_6)
- [EHW_DOC_1](../../spec/external_hw_docs.md#ehw_doc_1), Chapter 4.8.7

Status: approved
