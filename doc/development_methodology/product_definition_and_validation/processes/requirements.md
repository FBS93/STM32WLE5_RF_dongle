# requirements

## Purpose

Define requirements from specifications.

## Input work products

- Specifications

## Output work products

- Requirements

## Steps

1. Review the specifications.
2. Identify the needs to be addressed by the requirements.
3. Define the requirements derived from the specifications.
4. Define traceability between requirements and specifications.
5. Check the requirements for completeness, consistency and correctness.

## Guidelines

### Requirements work product

Requirements shall be defined in a clear and unambiguous way. When input specifications are ambiguous or incomplete, the ambiguity shall not be propagated downstream and shall be resolved through an explicit technically viable proposal.

#### Requirements structure

The requirements shall be structured in the following documents:
- One or more `req_<functional_domain>.md` documents containing requirements for each identified functional domain. The requirements in these documents shall follow the format defined in the [Functional domain requirements](#functional-domain-requirements).
- `req_interfaces.md` containing requirements defining all interfaces. The requirements in this document shall follow the format defined in the [Interface requirements](#interface-requirements).
- `req_parameters.md` containing requirements defining all parameters. The requirements in this document shall follow the format defined in the [Parameter requirements](#parameter-requirements).

`<functional_domain>` identifies a functional grouping derived from the upstream elements, used to organize related requirements.

Each document may be organized into chapters to support hierarchical structuring as needed.

#### Functional domain requirements

Functional domain requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `REQ_<FUNC_DOMAIN>_<X>`
  - The heading level may vary depending on the document structure.
  - `<FUNC_DOMAIN>` identifies the functional domain and shall match the functional domain used in the document name where the requirement is defined.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A description using the following pattern: `<subject> shall <required behavior>.`
  - `<subject>` identifies the element that is responsible for fulfilling the requirement.
  - `<required behavior>` may include actions, conditions, constraints, timing, or any other information needed to define the requirement clearly.
- Validation criteria. Shall be defined as a list of one or more criteria describing how the requirement is validated in a clear and unambiguous way, including measurable values, tolerances, or observable behaviors when applicable.
- Domains to which the requirement applies. Shall be defined as a list of engineering domain abbreviations in lower case and shall specify all domains responsible for implementing or fulfilling the requirement.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- References to other requirements when applicable. Shall be defined as a list of Markdown links referencing the target requirement identifier.
- A status indicating the approval state of the requirement. Shall use exactly one of these values: `not approved`, `approved`.

The following functional domain requirement template shall be used:

```md
# REQ_<FUNC_DOMAIN>_<X>

<subject> shall <required behavior>.

Validation criteria:
- <criterion>

Domain:
- <engineering domain abbreviation>

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

References:
- [REQ_...](#...)

Status: <status>
```

#### Interface requirements

Interface requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `REQ_INTERFACE_<X>`
  - The heading level may vary depending on the document structure.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the interface, following the pattern: `reqi_<requirementInterfaceName>`.
  - `<requirementInterfaceName>` is the unique interface name written in camelCase.
- A description defining the interface. The description shall include all necessary information to fully specify the interface, written in a clear and concise manner.
- A domain attribute specifying the engineering domains to which the requirement applies. Shall be defined as a list of engineering domain abbreviations in lower case and shall specify all domains responsible for implementing or fulfilling the requirement.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- A status indicating the approval state of the interface. Shall use exactly one of these values: `not approved`, `approved`.

The following interface requirement template shall be used:

```md
# REQ_INTERFACE_<X>

Name: reqi_<requirementInterfaceName>

<Description>.

Domain:
- <engineering domain abbreviation>

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

#### Parameter requirements

Parameter requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `REQ_PARAMETER_<X>`
  - The heading level may vary depending on the document structure.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the parameter, following the pattern: `reqp_<requirementParameterName>`.
  - `<requirementParameterName>` is the unique parameter name written in camelCase.
- A description defining the parameter. The description shall include all necessary information to fully specify the parameter, written in a clear and concise manner, including its explicit value and units when applicable.
- A domain attribute specifying the engineering domains to which the requirement applies. Shall be defined as a list of engineering domain abbreviations in lower case and shall specify all domains responsible for implementing or fulfilling the requirement.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- A status indicating the approval state of the parameter. Shall use exactly one of these values: `not approved`, `approved`.

The following parameter requirement template shall be used:

```md
# REQ_PARAMETER_<X>

Name: reqp_<requirementParameterName>

<Description>.

Domain:
- <engineering domain abbreviation>

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

#### Rules

- All upstream traceable elements used as requirements input shall be traced to at least one requirements traceable element.
- All [interface requirements](#interface-requirements) shall be referenced by at least one [functional domain requirement](#functional-domain-requirements).
- All [parameter requirements](#parameter-requirements) shall be referenced by at least one [functional domain requirement](#functional-domain-requirements).
