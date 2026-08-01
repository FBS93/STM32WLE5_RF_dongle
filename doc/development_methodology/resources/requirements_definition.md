# Requirements definition

## Purpose

Define the common requirements definition across the engineering domains.

## Requirements structure

The requirements shall be structured in the following documents:
- One or more `<domain>_<functional_domain>.md` documents containing requirements for each identified functional domain. The requirements in these documents shall follow the format defined in the [Functional domain requirements](#functional-domain-requirements).
- `<domain>_interfaces.md` containing requirements defining all interfaces. The requirements in this document shall follow the format defined in the [Interface requirements](#interface-requirements).
- `<domain>_parameters.md` containing requirements defining all parameters. The requirements in this document shall follow the format defined in the [Parameter requirements](#parameter-requirements).

`<domain>` identifies the engineering domain using its abbreviation in lower case.

`<functional_domain>` identifies a functional grouping derived from the upstream elements, used to organize related requirements within the domain.

Each document may be organized into chapters to support hierarchical structuring as needed.

### Functional domain requirements

Functional domain requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_REQ_<FUNC_DOMAIN>_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<FUNC_DOMAIN>` identifies the functional domain and shall match the functional domain used in the document name where the requirement is defined.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A description using the following pattern: `<subject> shall <required behavior>.`
  - `<subject>` identifies the element that is responsible for fulfilling the requirement.
  - `<required behavior>` may include actions, conditions, constraints, timing, or any other information needed to define the requirement clearly.
- Validation criteria. Shall be defined as a list of one or more criteria describing how the requirement is validated in a clear and unambiguous way, including measurable values, tolerances, or observable behaviors when applicable.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- References to other requirements of the same engineering domain when applicable. Shall be defined as a list of Markdown links referencing the target requirement identifier.
- A status indicating the approval state of the requirement. Shall use exactly one of these values: `not approved`, `approved`.

The following functional domain requirement template shall be used:

```md
# <DOMAIN>_REQ_<FUNC_DOMAIN>_<X>

<subject> shall <required behavior>.

Validation criteria:
- <criterion>

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

References:
- [<DOMAIN>_...](#...)

Status: <status>
```

### Interface requirements

Interface requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_REQ_INTERFACE_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the interface, following the pattern: `<domain>i_<requirementInterfaceName>`.
  - `<domain>` identifies the engineering domain using its abbreviation in lower case.
  - `<requirementInterfaceName>` is the unique interface name written in camelCase.
- A description defining the interface. The description shall include all necessary information to fully specify the interface, written in a clear and concise manner.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- A status indicating the approval state of the interface. Shall use exactly one of these values: `not approved`, `approved`.

The following interface requirement template shall be used:

```md
# <DOMAIN>_REQ_INTERFACE_<X>

Name: <domain>i_<requirementInterfaceName>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

### Parameter requirements

Parameter requirements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_REQ_PARAMETER_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the parameter, following the pattern: `<domain>p_<requirementParameterName>`.
  - `<domain>` identifies the engineering domain using its abbreviation in lower case.
  - `<requirementParameterName>` is the unique parameter name written in camelCase.
- A description defining the parameter. The description shall include all necessary information to fully specify the parameter, written in a clear and concise manner, including its explicit value and units when applicable.
- Upstream traceability to one or more upstream elements. Shall be defined as a list of Markdown links referencing the corresponding upstream element.
- A status indicating the approval state of the parameter. Shall use exactly one of these values: `not approved`, `approved`.

The following parameter requirement template shall be used:

```md
# <DOMAIN>_REQ_PARAMETER_<X>

Name: <domain>p_<requirementParameterName>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

## Rules

- All upstream traceable elements used as requirements input shall be traced to at least one requirements traceable element.
- All [interface requirements](#interface-requirements) shall be referenced by at least one [functional domain requirement](#functional-domain-requirements).
- All [parameter requirements](#parameter-requirements) shall be referenced by at least one [functional domain requirement](#functional-domain-requirements).
