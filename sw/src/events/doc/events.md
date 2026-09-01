# SW_DETAILED_DESIGN_3

## events overview

`events` is a header-only software unit that defines the project-specific events.

Upstream traceability:
- [SW_ARCH_DESIGN_1](../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_INTERFACE_4](../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_INTERFACE_5](../../../arch/sw_interfaces.md#sw_arch_interface_5)

Hardware resources:
- None. This unit contains only platform-independent event declarations.

External dependencies:
- Event Driven Framework (EDF): Provides `EDF_event_t` as the common event base and signal type.
- [rf_types](../../rf_types/doc/rf_types.md): Provides project-specific shared RF types.

Status: not approved

## Glossary

| Term | Definition |
|---|---|
| Event signal | EDF signal identifying one project-specific event. |

## Usage example

Active objects shall include [`events.h`](../inc/events.h) to use its signal enumerators and event types.
