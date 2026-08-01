# Cross-domain interface definition

## Purpose

Define the common cross-domain interface definition across the engineering domains.

## Cross-domain interface structure

Cross-domain interfaces shall be documented in dedicated Markdown documents following the naming pattern: `<source_domain>_<target_domain>_interface.md`

`<source_domain>` identifies the engineering domain that defines the interface-relevant information using its abbreviation in lower case.

`<target_domain>` identifies the engineering domain affected by that information using its abbreviation in lower case.

Each document may be organized into chapters to support hierarchical structuring as needed.

### Cross-domain interface elements

Cross-domain interface elements shall have the following attributes:
- A unique identifier defined as a Markdown heading, following the pattern: `<SOURCE_DOMAIN>_<TARGET_DOMAIN>_INTERFACE_<X>`
  - The heading level may vary depending on the document structure.
  - `<SOURCE_DOMAIN>` identifies the source engineering domain using its abbreviation in upper case.
  - `<TARGET_DOMAIN>` identifies the target engineering domain using its abbreviation in upper case.
  - `<X>` is a monotonically increasing number that shall never be reused.
- A description defining the element. The description shall include all necessary information to fully specify the element for the target domain, written in a clear and concise manner.
- Upstream traceability to one or more source domain traceable elements. Shall be defined as a list of Markdown links referencing the corresponding source domain traceable elements.
- A status indicating the approval state of the cross-domain interface element. Shall use exactly one of these values: `not approved`, `approved`.

The following element template shall be used:

```md
# <SOURCE_DOMAIN>_<TARGET_DOMAIN>_INTERFACE_<X>

<Description>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```

## Rules

- All cross-domain interface elements shall be traced to at least one source domain traceable element.
- All source domain traceable elements relevant to the target domain shall be captured by at least one [cross-domain interface element](#cross-domain-interface-elements).
