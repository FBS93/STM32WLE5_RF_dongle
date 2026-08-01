# specs-definition

## Purpose

Define project specifications from stakeholder specifications.

## Input work products

- Stakeholder specifications

## Output work products

- Specifications
- Tools
- External hardware documentation

## Steps

1. Review the stakeholder specifications.
2. Identify specification elements by extracting the smallest meaningful text fragments.
3. Define the specifications.
4. Check the specifications for completeness, consistency and correctness.
5. Identify tools available for project development, validation and debugging.
6. Define the tools.
7. Check the tools for completeness, consistency and correctness.
8. Identify external hardware documentation for hardware used by the project’s target product.
9. Define the external hardware documentation.
10. Check the external hardware documentation for completeness, consistency and correctness.

## Guidelines

### Specifications work product

A dedicated `specs.md` document shall be defined to capture all specifications relevant to the system design by decomposing stakeholder specifications into traceable specifications.

Each specification of the `specs.md` document shall:
- Be derived directly from stakeholder specifications without modifying their original content.
- Be defined by extracting the smallest possible meaningful text fragments.
- Preserve the original text exactly as defined in stakeholder specifications.
- Ensure each specification element is self-contained and understandable.
- Define a unique identifier for each specification element following the pattern: `SPEC_<X>`
  - `<X>` is a monotonically increasing number that shall never be reused.
- Define a milestone identifying the V-model milestone to which the specification element belongs following the pattern: `M_<X>`
  - `<X>` identifies the milestone number and may be shared by multiple specification elements belonging to the same milestone.
- Define a status indicating the approval state of the specification. Shall use exactly one of these values: `not approved`, `approved`.

The following specification template shall be used:

```md
# SPEC_<X>

<Exact text extracted from stakeholder specifications>.

Milestone: M_<X>

Status: <status>
```

### Tools work product

A dedicated `tools.md` document shall be defined to capture all project-specific tools for project implementation, test and debugging by reviewing stakeholder specifications.

Each tool of the `tools.md` document shall:
- Define all project-specific tools available for implementation, test and debugging.
- Describe how each tool shall be used.
  - Reference how it is integrated into the [Embedded Workbench](../../../ew.md) when applicable instead of redefining it.
  - Reference tool-specific manuals or stakeholder documentation that support its usage.
- Not redefine tools already provided by Embedded Workbench.

The following tool template shall be used:

```md
# <Tool name>

## Overview

<Description of the tool and the cases in which it is useful>.

## Usage

<Usage description. May include references to manufacturer manuals or stakeholder specifications explaining how to use the tool>.
```

### External hardware documentation work product

A dedicated `external_hw_docs.md` document shall be defined to capture stakeholder-provided documentation for external hardware used by the project's target product by reviewing stakeholder specifications.

Each external hardware documentation of the `external_hw_docs.md` document shall:
- Define a unique identifier for each external hardware documentation entry following the pattern: `EHW_DOC_<X>`
  - `<X>` is a monotonically increasing number that shall never be reused.
- Provide a brief description of the external hardware documentation.
- Provide a link to the stakeholder-provided external hardware documentation.

The following external hardware documentation template shall be used:

```md
# EHW_DOC_<X>

<Brief description of the external hardware documentation>.

Link: <link to stakeholder-provided document>
```
