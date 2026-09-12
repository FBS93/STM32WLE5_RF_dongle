# SW_DETAILED_DESIGN_1

## host_command_manager overview

`host_command_manager` receives host commands, publishes the corresponding EDF request events, and forwards subscribed response events to the host.

The internal `host_command_manager_hal`:
- Has mutually exclusive target and host implementations selected by the software-unit build. The target implementation abstracts USART2; the host implementation minimally realizes the same functional interface.
- Have a RX FIFO capacity of 264 bytes, and a TX FIFO capacity of 267 bytes.
- Publishes `HOST_DATA_AVAILABLE` from its RX ISR only when data arrives while the RX FIFO is empty.
- `hostCommandManagerHal_write()` enqueues the data and, if the TX FIFO was empty, enables the TX interrupt; the USART2 ISR drains the FIFO while the hardware accepts data and disables the TX interrupt when the FIFO becomes empty.

The active object implementation:
- Drains the RX FIFO into persistent frame-assembly state and retains trailing incomplete frames.
- Publishes a request event only after receiving and validating the corresponding complete command.
- Writes complete encoded responses through the `host_command_manager_hal` API.
- Owns the RX assembly frame buffer with a capacity of 264 bytes (the largest request contains the 255-byte TX Packet Payload plus the six decoded-frame overhead bytes and the worst-case COBS overhead and delimiter).

All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the Embedded Assert Framework (EAF) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.

Upstream traceability:
- [SW_ARCH_COMPONENT_2](../../../arch/sw_components.md#sw_arch_component_2)
- [SW_ARCH_COMPONENT_5](../../../arch/sw_components.md#sw_arch_component_5)
- [SW_ARCH_INTERFACE_1](../../../arch/sw_interfaces.md#sw_arch_interface_1)
- [SW_ARCH_INTERFACE_4](../../../arch/sw_interfaces.md#sw_arch_interface_4)
- [SW_ARCH_DESIGN_1](../../../arch/sw_designs.md#sw_arch_design_1)
- [SW_ARCH_DESIGN_2](../../../arch/sw_designs.md#sw_arch_design_2)

Hardware resources:
- USART2 on PA2/PA3: Receives host commands and transmits responses and reports through [HW_SW_INTERFACE_7](../../../../hw/cross_domain_interfaces/hw_sw_interface.md#hw_sw_interface_7).

External dependencies:
- Embedded Assert Framework (EAF): Provides assertions to enforce Design by Contract (DbC) in all undefined fault handling cases.
- Embedded Base Framework (EBF): Provides critical-section handling used directly by the USART2 HAL.
- Event Driven Framework (EDF): Provides the active-object HSM and publish-subscribe event delivery.
- Embedded Middleware Framework (EMF): Provides COBS encoding/decoding, CRC-16/XMODEM, little-endian conversion, and byte FIFO buffering for UART reception.
- [events](../../events/doc/events.md): Provides the shared project-specific event definitions.

Status: approved

## Glossary

| Term | Definition |
|---|---|
| RX frame assembly | Persistent encoded-frame bytes retained until a complete host command is available. |

## Usage example

```c
static hostCommandManager_t host_command_manager;

void EDF_onStartup(void)
{
  hostCommandManager_startup();
}

int main(void)
{
  EDF_init();

  // Initialize other application and EDF structures.

  hostCommandManager_init(&host_command_manager);

  EDF_run();
}
```
