# sw-architecture

## Purpose

Define software architecture from software requirements.

## Input work products

- Software requirements

## Output work products

- Software architecture

## Steps

1. Review the software requirements.
2. Identify the software architecture elements required to address the software requirements.
3. Define the software architecture based on the identified software architecture elements.
4. Define traceability between software architecture elements and software requirements.
5. Check the software architecture for completeness, consistency and correctness.

## Guidelines

### Software architecture work product

The software architecture work product shall follow the [Architecture definition](../../resources/architecture_definition.md).

#### Architecture model

The software architecture shall follow an event-driven model. The use of this architecture model with the [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md) shall be defined as a software design.

The software architecture shall be aligned with the capabilities and features of the [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md).

The software design defining the use of the event-driven architecture model shall contain:
- A justification of the selection of the event-driven model and the use of EDF with a fixed sentence.
- The definition of the complete set of available software events. Each event shall be specified with:
  - An event name in UPPER_SNAKE_CASE. The event name shall be unique, as it is used as the event identifier.
  - A description defining the event. The description shall include all necessary information to fully specify the event, written in a clear and concise manner.
  - Event parameters when applicable. Each parameter shall be specified with:
    - A parameter name in lower_snake_case. The parameter name shall be unique within the event, as it is used as the parameter identifier.
    - A description defining the parameter. The description shall include all necessary information to fully specify the parameter, written in a clear and concise manner.

The following template shall be used for the description of this software design:

```md
This architecture shall follow an event-driven model using Event Driven Framework (EDF).

## Events

### <EVENT_NAME>

<Event description>.

Parameters:
- <param_name>: <Parameter description>.
```

In this architecture model, all project-specific software components shall be defined as active objects. Each active object shall:
- Encapsulate its own state and behavior.
- Process events asynchronously without blocking or sharing resources with other software components.

Hint: If a shared resource is identified, the solution might be to isolate it in a dedicated active object and access it from other active objects through events.

Events, using the EDF publish/subscribe mechanism, shall be the only way of interaction between software components. Direct function calls between software components are not permitted.

A software component named `EDF` shall be defined and linked to the software design defining the use of the event-driven architecture model. This software component shall be used in the architecture views to represent the unique way of interaction between project-specific software components.

Each software component shall use a single architecture interface to `EDF` software component. The description of this interface shall contain:
- A list of published events.
- A list of subscribed events.

Events in both lists shall be referenced only by their `EVENT_NAME`, which is the unique identifier of the event defined in the software design defining the use of the event-driven architecture model.

The following template shall be used for the description of each interface used for a given software component:

```md
Published events:
- <EVENT_NAME>
- <EVENT_NAME>

Subscribed events:
- <EVENT_NAME>
- <EVENT_NAME>
```

#### Platform

The software architecture shall define the platform as a software design.

The platform software design shall define the platform-specific architectural decisions and default configurations.

All platform characteristics required to define the software architecture shall be provided by upstream elements. If relevant platform characteristics are not defined by upstream elements and no valid default option is available, the software architecture process shall not be completed until the missing information is defined upstream.

The platform software design shall contain:
- Toolchains:
  - If the target toolchain is not explicitly defined by upstream elements and the MCU architecture is ARM, the default option for the target shall be the [arm_gcc.cmake](../../../../tools/cmake/toolchains/arm_gcc.cmake) toolchain.
  - If the host toolchain is not explicitly defined by upstream elements, the default option for the host shall be the [linux.cmake](../../../../tools/cmake/toolchains/linux_gcc.cmake) toolchain.
- Build configuration:
  - If upstream elements do not specify the build configuration and the MCU architecture is ARM, the default build presets shall be:
    - `host`: Host execution (SiL) in release configuration.
    - `host_debug`: Host execution (SiL) in debug configuration.
    - `host_hard_debug`: Host execution (SiL) in debug configuration with preprocessed source code available for inspection.
    - `target_<platform>`: Target embedded execution in release configuration.
    - `target_<platform>_debug`: Target embedded execution in debug configuration.
    - `target_<platform>_hard_debug`: Target embedded execution in debug configuration with preprocessed source code available for inspection.
  - If build flags for the selected toolchain are not explicitly defined or are not self-evident, they shall be defined here, including the presets to which they apply.
- Memory layout:
  - A memory layout divided into RAM layout, ROM layout (e.g., program flash, ...) and NVM layout (e.g., persistent parameters, persistent data, ...). Each memory layout shall be represented as a Markdown table. Each row of this table represents a memory region of the memory layout and may describe any level of granularity, ranging from a memory block to individual parameters. Each memory region shall be specified with:
    - A memory region name in lower_snake_case. The memory region name shall be unique across all memory layouts, as it is used as the memory region identifier.
    - A description defining the memory region. The description shall include all necessary information to fully specify the memory region, written in a clear and concise manner.
    - The start address of the memory region.
    - The end address of the memory region.
- Debug configuration: 
  - Definition of the debug interfaces used (e.g., SWD, JTAG, etc.).
- [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md) integration:
  - Integration strategy for [Embedded Base Framework (EBF)](../../../../sw/ecf/embedded_base_framework/doc/ebf.md) stdin/stdout integration. Upstream elements shall define the interface used for this integration. If no further detail is specified by upstream elements, the default shall be:
    - Stdin listener shall be executed in the reception ISR context.
    - Stdout shall use buffered transmission handled by ISR.
    - Stdout buffer size shall be configurable and defined according to the memory constraints of the target platform.

The following template shall be used for the description of this software design:

```md
The platform shall be defined as follows:

## Toolchains

<Design decisions related to the selected toolchains>.

## Build configuration

<Design decisions related to the selected build configuration>.

## Memory layout

### RAM layout

| Name | Description | Start address | End address |
|---|---|---|---|
| <memory_region_name> | <Description>. | <Start address in hexadecimal format> | <End address in hexadecimal format> |

### ROM layout

| Name | Description | Start address | End address |
|---|---|---|---|
| <memory_region_name> | <Description>. | <Start address in hexadecimal format> | <End address in hexadecimal format> |

### NVM layout

| Name | Description | Start address | End address |
|---|---|---|---|
| <memory_region_name> | <Description>. | <Start address in hexadecimal format> | <End address in hexadecimal format> |

## Debug configuration

<Design decisions related to the selected debug configuration>.

## Embedded C Framework (ECF) integration

<Design decisions related to the Embedded C Framework (ECF) integration strategy>.
```
