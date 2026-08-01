---
name: sys-qualification-test
description: >
  Define and implement system qualification tests against system requirements to validate that the system implementation correctly realizes them.
  Trigger: When system integration test is completed and system qualification tests shall be defined and implemented.
---

## When to Use

- After system integration test is completed.
- When any system qualification test activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with system requirements (no inconsistencies).
- Shall ensure all system qualification tests are derived from system requirements by adding explicit traceability between them.
- Shall define and implement the system qualification tests against the system requirements to validate that the system implementation correctly realizes them.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval on key system qualification test decisions.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [system_domain_overview.md](../../../doc/development_methodology/system_domain/system_domain_overview.md)
- [sys-qualification-test.md](../../../doc/development_methodology/system_domain/processes/sys-qualification-test.md)
