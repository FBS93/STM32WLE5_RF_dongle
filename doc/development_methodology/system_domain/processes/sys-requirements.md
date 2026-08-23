# sys-requirements

## Purpose

Define system requirements from specifications.

## Input work products

- Specifications
- Tools
- External hardware documentation

## Output work products

- System requirements

## Steps

1. Review the specifications.
2. Identify the needs to be addressed by the system requirements.
3. Define the system requirements derived from the specifications.
4. Define traceability between system requirements and specifications.
5. Check the system requirements for completeness, consistency and correctness.

## Guidelines

### System requirements work product

The system requirements work product shall follow the [Requirements definition](../../resources/requirements_definition.md).

System requirements shall be defined in a clear and unambiguous way. When input specifications are ambiguous or incomplete, the ambiguity shall not be propagated downstream and shall be resolved through an explicit technically viable proposal.

#### Domain attribute

System requirements shall define an additional domain attribute to specify the engineering domains to which the requirement applies.

The domain attribute shall:
- Be defined as a list of engineering domain abbreviations in lower case.
- Specify all domains responsible for implementing or fulfilling the requirement.
- Be used to support the identification of the engineering domains applicable to the project.
- Be used to support downstream allocation of system requirements to engineering domain requirements.

The domain attribute shall be included in all system requirements after the requirement-specific content and before the upstream traceability attribute, as shown in the following requirement template:

```md
# SYS_REQ_<FUNC_DOMAIN>_<X>

...

Domain:
- <engineering domain abbreviation>

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

...

```
