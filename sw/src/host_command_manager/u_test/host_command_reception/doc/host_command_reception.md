# SW_UNIT_TEST_1

## host_command_reception overview

Verify reception and validation of host command frames by the isolated HOST_DATA_AVAILABLE activity: assembly across invocations, concatenated commands, maximum TX_PACKET payload and rejection of integrity or structural framing errors.

Upstream traceability:
- [SW_DETAILED_DESIGN_1](../../../doc/host_command_manager.md#sw_detailed_design_1): RX draining, incomplete-frame retention, maximum request support and publication only after complete-command validation.

Status: approved

## Use case

Invoke only the [HOST_DATA_AVAILABLE activity](../../../inc/host_command_manager_activities.h) with TX_PACKET input following the DD's inherited [framing and integrity](../../../../../../sys/req/sys_interfaces.md#framing-and-integrity) and [command contract](../../../../../../sys/req/sys_interfaces.md#tx-packet-request). The maximum payload follows [SYS_REQ_PARAMETER_40](../../../../../../sys/req/sys_parameters.md#sys_req_parameter_40).

ETF_setUp resets FIFO, fake/history, allocation and snapshot state before each case. EFF isolates HAL I/O and EDF allocation/publication; real EMF provides FIFO, COBS, CRC and little-endian utilities. Three typed allocation slots remain valid independently of separate publication snapshots. Each case owns fresh manager assembly state.

## Verification scope

Check RX draining, publication only for complete valid commands, exact publication multiplicity and intact event signal, semantic payload length and bytes under the DD's [event mapping](../../../../../arch/sw_designs.md#sw_arch_design_1). Concatenated complete commands are checked as an unordered set; completion of their trailing fragment adds only its corresponding command. An incorrect CRC must produce neither publication nor host output under the inherited [command-processing contract](../../../../../../sys/req/sys_interfaces.md#command-processing). TX_PACKET produces no host response.

The eight implemented cases cover assembly/concatenation, a 255-byte payload, CRC rejection, declared payload lengths smaller and larger than the bytes present with otherwise correct CRC/COBS, malformed COBS, and valid COBS decoding to fewer than the minimum six frame bytes. Rejected frames drain RX without publishing an event or writing host output. Subsequent recovery, oversized input and worst-case COBS expansion are not covered. No oracle depends on internal assembly fields, allocation count, timing or pointer identity. The bounded fixture does not model general event pools. AO initialization, scheduler/full HSM, real EDF delivery, physical UART and RF operation are outside scope; EMF utilities are dependencies, not independently validated here. The same source is registered for SiL and HiL. Configuration-specific validation is complementary in [host_command_set_config](../../host_command_set_config/doc/host_command_set_config.md).
