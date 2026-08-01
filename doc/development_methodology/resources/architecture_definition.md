# Architecture definition

## Purpose

Define the common architecture definition across the engineering domains.

## Architecture structure

The architecture shall be structured in the following documents:
- `<domain>_components.md` containing components of the architecture. The components in this document shall follow the format defined in the [Architecture components](#architecture-components).
- `<domain>_interfaces.md` containing interfaces defining the interactions between components. The interfaces in this document shall follow the format defined in the [Architecture interfaces](#architecture-interfaces).
- `<domain>_parameters.md` containing parameters defining configurable or fixed values affecting the behavior. The parameters in this document shall follow the format defined in the [Architecture parameters](#architecture-parameters).
- `<domain>_designs.md` containing design decisions of the architecture. The design decisions in this document shall follow the format defined in the [Architecture designs](#architecture-designs).

`<domain>` identifies the engineering domain using its abbreviation in lower case.

Each document may be organized into chapters to support hierarchical structuring as needed.

### Architecture components

Architecture components shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_ARCH_COMPONENT_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the component, following the pattern: `<domain>c_<architectureComponentName>`.
  - `<domain>` identifies the engineering domain using its abbreviation in lower case.
  - `<architectureComponentName>` is the unique component name written in camelCase.
- A description defining the component. The description shall include all necessary information to fully specify the component, written in a clear and concise manner. When other architecture elements are referenced from the architecture component, their usage shall be explicitly defined (i.e., referenced interfaces should specify how they are used by the component and, when applicable, how they are implemented by it; referenced parameters should specify their effect on the component behavior).
- Upstream traceability to one or more requirements. Shall be defined as a list of Markdown links referencing the corresponding requirements.
- References to other architecture elements when applicable. Shall be defined as a list of Markdown links referencing the target element identifier.
- A status indicating the approval state of the component. Shall use exactly one of these values: `not approved`, `approved`.

The following component template shall be used:

```md
# <DOMAIN>_ARCH_COMPONENT_<X>

Name: <domain>c_<architectureComponentName>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

References:
- [<DOMAIN>_...](#...)

Status: <status>
```

### Architecture interfaces

Architecture interfaces shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_ARCH_INTERFACE_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the interface, following the pattern: `<domain>i_<architectureInterfaceName>`.
  - `<domain>` identifies the engineering domain using its abbreviation in lower case.
  - `<architectureInterfaceName>` is the unique interface name written in camelCase.
- A description defining the interface. The description shall include all necessary information to fully specify the interface, written in a clear and concise manner.
- Upstream traceability to one or more requirements. Shall be defined as a list of Markdown links referencing the corresponding requirements.
- A status indicating the approval state of the interface. Shall use exactly one of these values: `not approved`, `approved`.

The following interface template shall be used:

```md
# <DOMAIN>_ARCH_INTERFACE_<X>

Name: <domain>i_<architectureInterfaceName>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

If an interface is directly inherited from an upstream traceable element used as architectural input, an architecture interface shall be created with a single upstream traceability reference to that element and a fixed interface description as follows:

```md
# <DOMAIN>_ARCH_INTERFACE_<X>

Interface inherited from upstream definition.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)
```

### Architecture parameters

Architecture parameters shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_ARCH_PARAMETER_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the parameter, following the pattern: `<domain>p_<architectureParameterName>`.
  - `<domain>` identifies the engineering domain using its abbreviation in lower case.
  - `<architectureParameterName>` is the unique parameter name written in camelCase.
- A description defining the parameter. The description shall include all necessary information to fully specify the parameter, written in a clear and concise manner, including its explicit value and units when applicable.
- Upstream traceability to one or more requirements. Shall be defined as a list of Markdown links referencing the corresponding requirements.
- A status indicating the approval state of the parameter. Shall use exactly one of these values: `not approved`, `approved`.

The following parameter template shall be used:

```md
# <DOMAIN>_ARCH_PARAMETER_<X>

Name: <domain>p_<architectureParameterName>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

If a parameter is directly inherited from an upstream traceable element used as architectural input, an architecture parameter shall be created with a single upstream traceability reference to that element and a fixed parameter description as follows:

```md
# <DOMAIN>_ARCH_PARAMETER_<X>

Parameter inherited from upstream definition.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)
```

### Architecture designs

Architecture designs shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<DOMAIN>_ARCH_DESIGN_<X>`
  - The heading level may vary depending on the document structure.
  - `<DOMAIN>` identifies the engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A unique and descriptive name for the design, following the pattern: `<domain>d_<architectureDesignName>`.
  - `<domain>` identifies the engineering domain using its abbreviation in lower case.
  - `<architectureDesignName>` is the unique design name written in camelCase.
- A description defining the design. The description shall include all necessary information to fully specify the architectural design decision, written in a clear and concise manner. A design captures architectural decisions that are not represented as components, interfaces, or parameters.
- Upstream traceability when applicable. Shall be defined as a list of Markdown links referencing the corresponding requirement.
- A status indicating the approval state of the design. Shall use exactly one of these values: `not approved`, `approved`.

The following design template shall be used:

```md
# <DOMAIN>_ARCH_DESIGN_<X>

Name: <domain>d_<architectureDesignName>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

## Architecture views

All views shall be defined using [Mermaid](https://github.com/mermaid-js/mermaid).

The architecture shall be represented using the following mandatory views:
- One static view using a component diagram. All architecture components and architecture interfaces shall appear in this view.
- One or more dynamic views using sequence diagrams representing relevant interaction use cases between architecture components. All architecture components and architecture interfaces shall appear in at least one of these views. The architecture parameters may appear if necessary in this type of diagrams.

Additional UML diagram types may be used when needed to represent architecture designs, providing additional context for design decisions. Their use is strongly recommended when appropriate.

When applicable, all diagram elements shall use the traceable identifiers of the architecture elements they represent.

Each diagram shall be defined in a dedicated Markdown document following the naming pattern: `<domain>_<diagram_name>_<diagram_type>.md`
- `<domain>` identifies the engineering domain using its abbreviation in lower case.
- `<diagram_name>` identifies the represented view and shall be unique within the domain.
- `<diagram_type>` identifies the diagram type (e.g., component_diagram, sequence_diagram, ...).

The following diagram document template shall be used:

```md
# <domain>_<diagram_name>_<diagram_type>

## Description

<Description>.

## Diagram

<mermaid diagram definition>
```

## Rules

- All upstream traceable elements used as architecture input shall be traced to at least one architecture traceable element.
- All [architecture interfaces](#architecture-interfaces) shall be referenced by at least one [architecture component](#architecture-components).
- All [architecture parameters](#architecture-parameters) shall be referenced by at least one [architecture component](#architecture-components).
