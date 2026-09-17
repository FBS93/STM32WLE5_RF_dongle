# SW_DETAILED_DESIGN_2

## rf_manager overview

`rf_manager` stores the current semantic RF configuration, controls the radio in response to subscribed EDF events, and publishes RF responses and reports.

The internal `rf_manager_hal`:
- Has mutually exclusive target and host implementations selected by the software-unit build. The target implementation abstracts the STM32WLE5 sub-GHz radio and RF switch behind the shared semantic API; the host implementation minimally realizes the same functional interface.
- Reports asynchronous TX, RX, and CAD indications through registered callbacks and provides received payload, packet status, and radio errors.

The active object implementation:
- Zero-initializes its configuration storage to establish defined C object values; this is not a semantically valid RF default.
- Returns the stored configuration contents for `GET_CONFIG_REQUEST` without requiring prior configuration.
- Stops any active RF operation before handling valid `SET_CONFIG`, `CALIBRATE`, `TX_PACKET`, `RX_START`, `CAD_START`, `TX_CONTINUOUS_WAVE`, `TX_CONTINUOUS_PREAMBLE`, or `STOP`; diagnostic and configuration queries do not stop it.
- Stores and atomically applies complete configurations, tracks the host-visible active RF operation, and maps radio callbacks to the traced result events.
- Discards radio-rejected packets without publishing `RX_PACKET` or completing the active RX or CAD operation.
- Accepts RX only for LoRa and FSK, CAD only for LoRa, and continuous-preamble TX only for LoRa, FSK, and MSK. The applicable activities return whether the modeled transition shall occur; rejected requests have no effect.
- The AO stores whether a valid `SET_CONFIG` has established its configuration; its activities exclusively manage and consult that state and do not apply TX, RX, CAD, continuous-wave, or continuous-preamble hardware operations before then. Calibration, stop, configuration queries, and diagnostics remain independent and available before configuration.
- Repeats CAD after a clear result and, after detection, either completes or receives according to the stored CAD exit mode.

All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the Embedded Assert Framework (EAF) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.

Upstream traceability:
- [SW_ARCH_COMPONENT_3](../../../arch/sw_components.md#sw_arch_component_3)
- [SW_ARCH_COMPONENT_5](../../../arch/sw_components.md#sw_arch_component_5)
- [SW_ARCH_INTERFACE_2](../../../arch/sw_interfaces.md#sw_arch_interface_2)
- [SW_ARCH_INTERFACE_5](../../../arch/sw_interfaces.md#sw_arch_interface_5)
- [SW_ARCH_PARAMETER_1](../../../arch/sw_parameters.md#sw_arch_parameter_1)
- [SW_ARCH_DESIGN_1](../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2)

Hardware resources:
- MCU-integrated sub-GHz radio: Used by `rf_manager` for system RF functionality; see [HW_SW_INTERFACE_9](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9).
- SUBGHZSPI: Used by `rf_manager_hal` to transfer commands and data over the MCU-internal connection to the integrated sub-GHz radio; see [HW_SW_INTERFACE_9](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_9).
- PA4 `RF_SWITCH_CTRL`: Used by `rf_manager` to control the RF front-end path; see [HW_SW_INTERFACE_10](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_10).
- Integrated-radio `RFO_HP` RF path: Used by `rf_manager` for the RF front-end PA path; see [HW_SW_INTERFACE_12](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_12).

External dependencies:
- Embedded Assert Framework (EAF): Provides assertions to enforce Design by Contract (DbC) in all undefined fault handling cases.
- Embedded Base Framework (EBF): Provides the core-selection definitions used directly by the radio ISR integration.
- Event Driven Framework (EDF): Provides the active-object HSM and publish-subscribe event delivery.
- [Embedded Middleware Framework (EMF)](../../../ecf/embedded_middleware_framework/doc/emf.md): Provides reusable utilities.
- [rf_types](../../rf_types/doc/rf_types.md): Provides project-specific shared RF types.
- [events](../../events/doc/events.md): Provides the shared project-specific event definitions.
- [Semtech SX126X driver v2.5.0](../../../third_party/sx126x_driver-2.5.0/README.md): Provides the low-level command API for the STM32WLE5-integrated sub-GHz radio.

Status: approved

## Glossary

| Term | Definition |
|---|---|
| RF operation | One of the host-visible packet TX, RX, CAD, continuous-wave TX, continuous-preamble TX, or calibration operations. |

## Usage example

```c
static rfManager_t rf_manager;

void EDF_onStartup(void)
{
  rfManager_startup();
}

int main(void)
{
  EDF_init();

  // Initialize other application and EDF structures.

  rfManager_init(&rf_manager);

  EDF_run();
}
```
