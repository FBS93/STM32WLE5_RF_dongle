# sw-detailed-design

## Purpose

Define software detailed design from software requirements and software architecture.

## Input work products

- Software requirements
- Software architecture

## Output work products

- Software detailed design

## Steps

1. Review the software requirements and software architecture.
2. Identify the software units required to address the software requirements and software architecture.
3. Define the detailed design of each software unit.
4. Define traceability between software detailed design and software architecture.
5. Check the software detailed design for completeness, consistency and correctness.

## Guidelines

### Software detailed design work product

Software detailed designs shall be derived from software architecture and software requirements.

Software requirements shall be used only as complementary input when the software architecture does not fully define the required behavior.

Software detailed designs shall not define direct traceability to software requirements.

All software architecture elements shall be traced to one or more software units. Each software architecture element may be refined into one or more software units (1:1 or 1:N) depending on the software detailed design needs.

In cases where a software component is refined into multiple software units (1:N), it is recommended to group the corresponding software units under a common folder named the same as the software component.

Each software unit shall use a descriptive `lower_snake_case` name. This name shall be the common base name for its folder, Markdown document, headers, source files and implemented library. When a software component is refined 1:1 as a software unit, the software unit name shall be derived by removing the architectural `swc_` prefix and converting the remaining name to `lower_snake_case`.

Software units shall be reused from existing libraries when available. Before creating a new software unit or implementing project-specific functionality within a software unit, it shall be verified whether the required functionality can be provided by [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md). If a specific implementation within a software unit is still required, it shall be assessed whether it can be fully or partially designed as reusable software for other projects and therefore implemented in [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md). Project-specific software implementation shall be minimized.

Each project-specific software unit shall have its own software detailed design and shall consist of:
- A Markdown document named `<sw_unit_name>.md` following the template defined in [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md#librarymodule-documentation) with the following additional information:
  - A unique identifier defined as a top Markdown heading (all other headings defined in the [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md#librarymodule-documentation) template shall be shifted one level below this top heading), following the pattern: `SW_DETAILED_DESIGN_<X>`
    - `<X>` is a monotonically increasing number that shall never be reused.
  - The `<Library/module name> overview` chapter placeholder shall be replaced with the software unit name used in the document name using the following pattern `<Software unit name> overview`. The chapter shall contain the following information:
    - A description defining the software unit. The description shall include all necessary information to fully specify the software unit, written in a clear and concise manner.
      - The description of software units with executable behavior shall document that all fault handling not explicitly defined by upstream elements shall be handled using a Design by Contract (DbC) approach with the [Embedded Assert Framework (EAF)](../../../../sw/ecf/embedded_assert_framework/doc/eaf.md).
    - Upstream traceability to one or more software architecture elements. Shall be defined as a list of Markdown links referencing the corresponding software architecture elements.
    - A list of hardware resources used directly by the software unit, including their usage.
    - A list of external library dependencies, including their usage.
  - A status indicating the approval state of the software unit detailed design. Shall use exactly one of these values: `not approved`, `approved`.
  - The remaining chapters of the [library/module documentation](../resources/embedded_c_coding_guidelines.md#librarymodule-documentation) template shall follow this project-specific content in their original order.
- An [MBSE](https://github.com/FBS93/mbse) project, when required, to define executable HSMs (Hierarchical State Machines) or activity diagrams. The MBSE project and model JSON files shall be the source of truth from which the models are visualized or executed.
- All header files of the software unit. One header file named `<sw_unit_name>.h` shall define the main public header file of the software unit. Additional header files, if any, shall be named following the pattern `<sw_unit_name>_<header_specific_suffix>.h` and are intended to support the internal structure and detailed design of the software unit. All header files of the software unit shall follow the rules defined in [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md).

The following additions to the Markdown document template shall be applied to project-specific software units, in addition to the template defined in [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md#librarymodule-documentation):

```md
# SW_DETAILED_DESIGN_<X>

## <Software unit name> overview

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Hardware resources:
- <Hardware resource>: <Short description of its usage>

External dependencies:
- <External library name>: <Short description of its usage>

Status: <status>

<Remaining chapters from the [library/module documentation](../resources/embedded_c_coding_guidelines.md#librarymodule-documentation) template, in their original order.>
```

For software units with executable behavior, the following text shall be inserted immediately after `<Description>.`:

```md
All undefined fault handling cases shall be handled using a Design by Contract (DbC) approach with the Embedded Assert Framework (EAF) as follows:
  - An assert shall be triggered for any unexpected software condition (e.g., invalid states, unexpected values, unexpected execution paths, invalid function parameters, ...).
  - Upon assert trigger, the software shall enter an infinite loop with a critical section active to prevent further execution.
```

For software units with executable behavior, the following dependency shall also be included under `External dependencies`:

```md
- Embedded Assert Framework (EAF): Provides assertions to enforce Design by Contract (DbC) in all undefined fault handling cases.
```

#### Active object detailed design

If the software architecture specifies an event-driven model as a software design, all software components defined in the architecture as active objects shall be refined 1:1 into software units. A dedicated software unit named `events` shall be defined as header-only, containing all events defined in this software design, so that it is available to all active objects and can be used during implementation. This does not preclude the definition of other types of software units derived from other software architecture elements.

The software detailed design of software units implemented as active objects shall not duplicate information already defined in the corresponding software component. Instead, it shall extend it where needed and define upstream traceability to the corresponding software component.

The software detailed design of software units implemented as active objects shall be aligned with the capabilities and features of the [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md). As this framework will be used for their implementation, it shall be listed as a dependency in the software detailed design.

The software detailed design of a software unit implemented as an active object shall contain the following information in addition to the generic software unit detailed design:
- The definition of its Hierarchical State Machine (HSM) and activities using [MBSE](https://github.com/FBS93/mbse).
  - The HSM definition shall be kept as simple as possible. It shall only extract event parameters and apply minimal logic to trigger specific functions (activities) based on the received event. Functional logic shall not be implemented directly in the HSM.
  - All activities and states defined in the HSM shall correspond to function names to be implemented. These function names shall follow the naming conventions defined in [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md#naming-conventions).
  - Activities defined in the HSM shall be declared in a header file named `<sw_unit_name>_activities.h` to allow unit testing when required.

#### Platform package detailed design

If the software architecture specifies the platform as a software design, a software unit of type platform package shall be defined to implement all platform-reusable aspects. Project-dependent platform aspects of the platform software design shall not be implemented in the platform package and shall instead be implemented in project-specific software units with established traceability to them.

Platform package implementations shall be reused from [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md) when available. If not available, they shall be implemented as reusable software units within ECF rather than as project-specific software units.

As platform packages are not project-specific software units, their software detailed design shall be derived from the corresponding platform software design and upstream elements as a reusable and generic solution and shall not define project-specific traceability.

A platform package shall be identified by `<platform_name>`, which shall be a descriptive identifier reflecting the scope of the platform package (e.g., MCU family, board, execution environment, ...).

A software unit named `stdio` shall be defined within the platform package to implement [Embedded Base Framework (EBF)](../../../../sw/ecf/embedded_base_framework/doc/ebf.md) stdin/stdout integration as specified in the platform software design.

The content structures of the platform package and the `stdio` software unit deviate from the standard project-specific software unit structure. Their software detailed design shall consist only of the Markdown document based on the template defined in [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md#librarymodule-documentation).

The [STM32F103C8Tx platform package](../../../../sw/ecf/platform/stm32f103c8tx/doc/stm32f103c8tx_platform_package.md) shall be used as a reference for platform package detailed designs.
