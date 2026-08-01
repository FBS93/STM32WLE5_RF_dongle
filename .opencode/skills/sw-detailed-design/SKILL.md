---
name: sw-detailed-design
description: >
  Define software detailed design from software requirements and software architecture.
  Trigger: When software requirements and software architecture are approved and software detailed design shall be defined.
---

## When to Use

- After software requirements and software architecture are approved.
- When any software detailed design activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with software requirements and software architecture (no inconsistencies).
- Shall ensure all software detailed design is derived from software architecture by adding explicit traceability between them.
- Shall prioritize reuse over creation.
- Shall avoid embedding implementation details beyond what is required for software detailed design.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall identify and list all opportunities for reuse and ask for user decision before classifying them as reusable or project-specific.
- Shall ask for approval on key software detailed design decisions.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [software_domain_overview.md](../../../doc/development_methodology/software_domain/software_domain_overview.md)
- [sw-detailed-design.md](../../../doc/development_methodology/software_domain/processes/sw-detailed-design.md)
