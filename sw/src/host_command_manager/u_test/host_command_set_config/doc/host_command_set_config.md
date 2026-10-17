# SW_UNIT_TEST_2

## host_command_set_config overview

Verify SET_CONFIG structural validation, complete LoRa/FSK semantic event translation and selected high-value cross-field constraints through the isolated HOST_DATA_AVAILABLE activity.

Upstream traceability:
- [SW_DETAILED_DESIGN_1](../../../doc/host_command_manager.md#sw_detailed_design_1): publish requests only after complete-command validation and drain RX.
- [HOST_DATA_AVAILABLE activity](../../../inc/host_command_manager_activities.h): individual activity identified by the [DD model](../../../model/host_command_manager_hsm.md).

Status: approved

## Use case

Supply contract-ordered [configuration payloads](../../../../../../sys/req/sys_interfaces.md#configuration-payload) in SET_CONFIG frames with correct external length, CRC and COBS. The DD inherits the [SET_CONFIG event mapping](../../../../../arch/sw_designs.md#sw_arch_design_1) and the [discard-without-response contract](../../../../../../sys/req/sys_interfaces.md#command-processing). The valid LoRa and FSK controls use distinct nontrivial values, including signed power, multi-byte timers and all eight generic sync-word bytes.

ETF_setUp directly resets the real FIFO, EFF fakes/history, allocation slots and independent publication snapshots before each ETF case. EFF isolates HAL I/O and EDF allocation/publication; EMF supplies real FIFO, endian, CRC and COBS operations. Four union-backed allocation slots accommodate SET_CONFIG and the largest request without requiring allocator count, exact size or pointer identity. Every activity invocation starts with a fresh manager except where the reception suite explicitly tests assembly. Default ETF teardown/exit suffice; there are no acquired resources.

## Verification scope

| ETF case | Objective and inherited DD contract |
|---|---|
| `validLoraPublishesCompleteConfiguration` | One SET_CONFIG publication with every common and selected LoRa semantic member correctly translated. |
| `validFskPublishesCompleteConfiguration` | One SET_CONFIG publication with every common and selected FSK semantic member correctly translated; compatible modulation control. |
| `reservedPacketTypeIsDiscarded` | Reject undefined type 0x04 under [SYS_REQ_PARAMETER_1](../../../../../../sys/req/sys_parameters.md#sys_req_parameter_1). |
| `truncatedLoraBlockIsDiscarded` / `extraFskBlockByteIsDiscarded` | Enforce exactly one complete selected fixed block, independently of external frame length validation. |
| `sf5AndSf6AcceptTwelveSymbolPreamble` / `sf5AndSf6RejectElevenSymbolPreamble` | For each SF5/SF6, accept 12 symbols and reject 11, changing only preamble relative to its control; [SYS_REQ_PARAMETER_13](../../../../../../sys/req/sys_parameters.md#sys_req_parameter_13). |
| `fskRejectsInsufficientReceiveBandwidth` | Change only RX BW from 156.2 to 93.8 kHz, below BR + 2D even before frequency error. |
| `fskRejectsIncompatibleBitRate` | Change only BR from 50000 to 150000 bit/s at the valid control's deviation and RX BW. |
| `fskRejectsIncompatibleDeviation` | Change only deviation from 0x006666 to 0x01999A PLL steps at the valid control's BR and RX BW. |

The FSK oracle follows [SYS_REQ_PARAMETER_8](../../../../../../sys/req/sys_parameters.md#sys_req_parameter_8), [SYS_REQ_PARAMETER_9](../../../../../../sys/req/sys_parameters.md#sys_req_parameter_9) and [SYS_REQ_PARAMETER_11](../../../../../../sys/req/sys_parameters.md#sys_req_parameter_11), not an implementation predicate. The control's 50000 bit/s and 0x006666 deviation (~24999.62 Hz) occupy ~99999.24 Hz before frequency error, comfortably below 156200 Hz. [HW_SW_INTERFACE_8](../../../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_8) identifies the 32 MHz TCXO; Y1's [manufacturer datasheet](https://ecsxtal.com/store/pdf/ECS-TXO-25CSMV.pdf), p. 1, specifies ±2 ppm initial tolerance, ±2 ppm temperature stability for option D, ±0.2 ppm supply, ±0.2 ppm load and ±1 ppm first-year aging: even their conservative sum (5.4 ppm, 172.8 Hz at 32 MHz, doubled to 345.6 Hz by the contract) is negligible against the control's ~56.2 kHz margin. No acceptance boundary depends on a copied production HSE-error constant or an unspecified lifetime aging extrapolation. The three rejected variants occupy respectively ~100, ~200 and ~250 kHz against 93.8, 156.2 and 156.2 kHz. Each field remains individually valid and each variant also satisfies the weaker deviation + BR/2 constraints, isolating RX bandwidth compatibility.

All ten ETF cases check RX draining and absence of host output; rejection cases check absence of publication. Translation checks actual publication snapshots member by member, not padding, inactive union arms or a preloaded allocator oracle. The two SF cases each exercise two independent inputs, not recovery or publication order. The fixture is bounded, not a general event-pool model. This is not a field-by-field parameter battery: MSK/BPSK translation, other cross-field constraints, RF execution, AO startup/full HSM, real EDF delivery and physical UART are outside scope. [host_command_reception](../../host_command_reception/doc/host_command_reception.md) covers transport assembly and structural framing separately. Both suites are registered for SiL and HiL; registration is not physical execution.
