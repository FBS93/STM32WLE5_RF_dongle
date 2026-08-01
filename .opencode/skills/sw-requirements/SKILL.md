---
name: sw-requirements
description: >
  Define software requirements from system requirements, system architecture and hardware-software interface.
  Trigger: When system requirements, system architecture and hardware-software interface are approved and software requirements shall be defined.
---

## When to Use

- After system requirements, system architecture and hardware-software interface are approved.
- When any software requirements activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with system requirements, system architecture and hardware-software interface (no inconsistencies).
- Shall ensure all software requirements are derived from system requirements, system architecture and hardware-software interface by adding explicit traceability between them.
- Shall avoid embedding architectural or implementation decisions in software requirements.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval when software requirements are incomplete, ambiguous, or conflicting.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [software_domain_overview.md](../../../doc/development_methodology/software_domain/software_domain_overview.md)
- [sw-requirements.md](../../../doc/development_methodology/software_domain/processes/sw-requirements.md)
