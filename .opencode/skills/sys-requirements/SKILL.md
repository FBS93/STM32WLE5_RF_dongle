---
name: sys-requirements
description: >
  Define system requirements from specifications.
  Trigger: When specifications are approved and system requirements shall be defined.
---

## When to Use

- After specifications are approved.
- When any system requirements activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with specifications (no inconsistencies).
- Shall ensure all system requirements are derived from specifications by adding explicit traceability between them.
- Shall avoid embedding architectural or implementation decisions in system requirements.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval when system requirements are incomplete, ambiguous, or conflicting.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [system_domain_overview.md](../../../doc/development_methodology/system_domain/system_domain_overview.md)
- [sys-requirements.md](../../../doc/development_methodology/system_domain/processes/sys-requirements.md)
