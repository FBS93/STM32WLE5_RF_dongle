---
name: sw-integration-test
description: >
  Define and implement software integration tests against software architecture to validate that the software detailed design and software implementation correctly realizes it.
  Trigger: When software unit test is completed and software integration tests shall be defined and implemented.
---

## When to Use

- After software unit test is completed.
- When any software integration test activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with software architecture (no inconsistencies).
- Shall ensure all software integration tests are derived from software architecture by adding explicit traceability between them.
- Shall define and implement the software integration tests against the software architecture to validate that the software detailed design and software implementation correctly realizes it.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval on key software integration test decisions.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [software_domain_overview.md](../../../doc/development_methodology/software_domain/software_domain_overview.md)
- [sw-integration-test.md](../../../doc/development_methodology/software_domain/processes/sw-integration-test.md)
