# SW_UNIT_TEST_3

## rf_operation_admission overview

Verify that individual RF request activities admit only operations supported by the established configuration, reject without effects, and replace a previous operation only when admitted.

Upstream traceability:
- [SW_DETAILED_DESIGN_2](../../../doc/rf_manager.md#sw_detailed_design_2): configuration validity, modulation eligibility, rejected requests without effects and stopping active RF before an admitted operation.
- [RF request activities](../../../inc/rf_manager_activities.h): RX, CAD, packet TX, continuous-wave TX and continuous-preamble TX, identified by the [DD model](../../../model/rf_manager_hsm.md).

Status: approved

## Use case

Invoke the five activities directly with typed [events](../../../../events/inc/events.h) and representative stored [RF configurations](../../../../rf_types/inc/rf_types.h), without initializing or starting the AO, scheduler or HSM. Each request starts with a distinct previous host-visible operation. EFF isolates radio HAL and the EDF allocation/publication dependencies needed by the linked activity object. ETF_setUp directly resets the manager, fakes, history and argument snapshots. Default ETF teardown/exit suffice; no resources are acquired. The production static library is linked using the same CMake pattern as the host-manager suites.

## Verification scope

| ETF case | Admission objective |
|---|---|
| `rxRequiresConfiguration` | Reject RX before configuration even with LoRa stored. |
| `rxAcceptsLoraAndFsk` | Admit both RX-capable modulations. |
| `rxRejectsMskAndBpsk` | Reject the two TX-only modulations despite valid configuration. |
| `cadRequiresConfiguration` | Reject CAD before configuration even with LoRa stored. |
| `cadAcceptsLora` / `cadRejectsFsk` | Distinguish CAD eligibility from RX eligibility. |
| `preambleRequiresConfiguration` | Reject preamble TX before configuration even with LoRa stored. |
| `preambleAcceptsLoraFskAndMsk` / `preambleRejectsBpsk` | Admit all three supported modulations, including MSK which RX rejects, but exclude BPSK. |
| `packetRequiresConfiguration` / `packetReplacesPreviousOperation` | Gate packet TX and forward the stored configuration, request payload bytes and actual request length. |
| `waveRequiresConfiguration` / `waveReplacesPreviousOperation` | Gate continuous-wave TX and replace an existing RX operation when configured. |

All thirteen cases verify stored semantic configuration and validity preservation. Rejections preserve the previous operation and cause no HAL or EDF calls, including no stop or reconfiguration; Boolean APIs return false. Admissions return true where applicable, select the corresponding host-visible operation, and call stop before exactly the requested HAL start. Configuration and payload arguments are observed synchronously, without requiring pointer identity, padding equality, or an order between state updates and HAL calls. Equivalent modulation inputs share simple loops, not an HSM traversal or recovery scenario. Configuration establishment/queries, RF callbacks, full AO behavior, parameter-combination batteries, real EDF delivery, hardware drivers and physical RF behavior are outside scope. The same suite is registered for SiL and HiL; registration does not imply physical execution.
