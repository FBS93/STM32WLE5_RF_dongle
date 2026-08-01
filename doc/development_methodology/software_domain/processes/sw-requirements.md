# sw-requirements

## Purpose

Define software requirements from system requirements, system architecture and hardware-software interface.

## Input work products

- System requirements
- System architecture
- Hardware-software interface

## Output work products

- Software requirements

## Steps

1. Review the system requirements, system architecture and hardware-software interface.
2. Identify the needs to be addressed by the software requirements.
3. Define the software requirements derived from the system requirements, system architecture and hardware-software interface.
4. Define traceability from software requirements to system requirements, system architecture and hardware-software interface.
5. Check the software requirements for completeness, consistency and correctness.

## Guidelines

### Software requirements work product

The software requirements work product shall follow the [Requirements definition](../../resources/requirements_definition.md).

System requirements that do not include `sw` in their domain attribute and system architecture elements not traced to such requirements shall not be considered as input for software requirements definition.

#### Platform requirements

A dedicated `sw_platform.md` requirements document shall be defined to capture platform-related requirements.

Platform requirements shall define platform constraints derived from upstream elements.

Platform requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `SW_REQ_PLATFORM_<X>`
  - The heading level may vary depending on the document structure.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A description using the following pattern: `<subject> shall <required behavior>.`
  - `<subject>` identifies the element that is responsible for fulfilling the requirement.
  - `<required behavior>` may include actions, conditions, constraints, timing, or any other information needed to define the requirement clearly.
- Validation criteria. Shall be defined as a list of one or more criteria describing how the requirement is validated in a clear and unambiguous way, including measurable values, tolerances, or observable behaviors when applicable.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- References to other requirements of the same engineering domain when applicable. Shall be defined as a list of Markdown links referencing the target requirement identifier.
- A status indicating the approval state of the platform requirement. Shall use exactly one of these values: `not approved`, `approved`.

The following platform requirement template shall be used:

```md
# SW_REQ_PLATFORM_<X>

<subject> shall <required behavior>.

Validation criteria:
- <criterion>

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

References:
- [SW_...](#...)

Status: <status>
```
