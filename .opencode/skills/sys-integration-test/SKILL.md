---
name: sys-integration-test
description: >
  Define and implement system integration tests against system architecture to validate that the system implementation correctly realizes it.
  Trigger: When all engineering domain processes defined in the V-model are completed and system integration tests shall be defined and implemented.
---

## When to Use

- After all engineering domain processes defined in the V-model are completed and validated.
- When any system integration test activity is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current process, inputs, outputs, rules and directly referenced work products.
- Shall align with system architecture (no inconsistencies).
- Shall ensure all system integration tests are derived from system architecture by adding explicit traceability between them.
- Shall define and implement the system integration tests against system architecture to validate that the system implementation correctly realizes it.
- Shall keep documentation DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall ask for approval on key system integration test decisions.
- Shall set produced/modified work products status to `not approved`.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [system_domain_overview.md](../../../doc/development_methodology/system_domain/system_domain_overview.md)
- [sys-integration-test.md](../../../doc/development_methodology/system_domain/processes/sys-integration-test.md)
