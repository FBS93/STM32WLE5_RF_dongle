---
name: spec-review
description: >
  Review specifications process work products before closure.
  Trigger: When a specifications process is ready to be closed and requires validation.
---

## When to Use

- Before closing any specifications process activity.
- When a full review of specifications process work products is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current task.
- Shall verify the following points only for specifications process work products with status `not approved`:
  - Ensure that all specifications process work products are complete and consistent.
  - Identify missing traceability, inconsistencies, or deviations from the defined specifications process.
  - Verify that documentation is DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall update the status of compliant reviewed specifications process work products to `approved`; work products that fail the review shall remain `not approved`.
- Shall clearly state whether the specifications process work products are approved and the process can be closed, or whether further work is needed.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [specifications_overview.md](../../../doc/development_methodology/specifications/specifications_overview.md)
