---
name: sw-unit-test
description: >
  Define and implement software unit tests against software detailed design to validate that the software implementation correctly realizes it.
  Trigger: When software implementation is completed and software unit tests shall be defined and implemented.
---

## When to Use

- After software implementation is completed.
- When any software unit test activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with software detailed design (no inconsistencies).
- Shall ensure all software unit tests are derived from software detailed design by adding explicit traceability between them.
- Shall define and implement the software unit tests against the software detailed design to validate that the software implementation correctly realizes it.
- Shall never define and implement the software unit tests against the software implementation.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval on key software unit test decisions.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [software_domain_overview.md](../../../doc/development_methodology/software_domain/software_domain_overview.md)
- [sw-unit-test.md](../../../doc/development_methodology/software_domain/processes/sw-unit-test.md)
