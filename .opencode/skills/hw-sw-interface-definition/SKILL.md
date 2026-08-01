---
name: hw-sw-interface-definition
description: >
  Define the hardware-software interface from system requirements, system architecture and external hardware documentation.
  Trigger: When system requirements and system architecture are approved, external hardware documentation is available, and hardware-software interface shall be defined.
---

## When to Use

- After system requirements and system architecture are approved and external hardware documentation is available.
- When any hardware-software interface definition activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with system requirements, system architecture and external hardware documentation (no inconsistencies).
- Shall ensure all hardware-software interfaces are derived from system requirements, system architecture and external hardware documentation by adding explicit traceability between them.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval when hardware-software interface elements are incomplete, ambiguous, or conflicting.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [hardware_domain_overview.md](../../../doc/development_methodology/hardware_domain/hardware_domain_overview.md)
- [hw-sw-interface-definition.md](../../../doc/development_methodology/hardware_domain/processes/hw-sw-interface-definition.md)
