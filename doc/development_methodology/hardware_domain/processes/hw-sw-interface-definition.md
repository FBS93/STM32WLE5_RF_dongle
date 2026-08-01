# hw-sw-interface-definition

## Purpose

Define the hardware-software interface from system requirements, system architecture and external hardware documentation.

## Input work products

- System requirements
- System architecture
- External hardware documentation

## Output work products

- Hardware-software interface

## Steps

1. Review the system requirements, system architecture and external hardware documentation.
2. Identify the externally provided hardware information relevant to the software domain.
3. Define the hardware-software interface derived from the system requirements, system architecture and external hardware documentation.
4. Define traceability from hardware-software interface to system requirements, system architecture and external hardware documentation.
5. Check the hardware-software interface for completeness, consistency and correctness.

## Guidelines

### Hardware-software interface work product

The hardware-software interface work product shall follow the [Cross-domain interface definition](../../resources/cross_domain_interface_definition.md).

System requirements that do not include `hw` in their domain attribute and system architecture elements not traced to such requirements shall not be considered as input for hardware-software interface definition.

System requirements and system architecture shall provide the required behavior and architectural context for identifying relevant external hardware information. External hardware documentation shall provide the relevant information needed to define the hardware-software interface.

Each hardware-software interface element shall define upstream traceability to:
- One or more system requirements and/or system architecture, that justify its relevance.
- One or more external hardware documentation that provide the hardware source information.

The hardware-software interface work product may include, for example, when applicable:
- Programmable device interfaces and peripherals relevant to software implementation.
- External and internal interrupt sources and the conditions that trigger them relevant to software implementation.
- Boot, reset, programming and debug interfaces.
- Register-level configuration information required to initialize and operate the used peripherals, interfaces and hardware features.
