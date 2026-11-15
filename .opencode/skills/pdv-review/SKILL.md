---
name: pdv-review
description: >
  Review product definition and validation process work products before closure.
  Trigger: When a product definition and validation process is ready to be closed and requires validation.
---

## When to Use

- Before closing any product definition and validation process activity.
- When a full review of product definition and validation process work products is required.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current task.
- Shall verify the following points only for product definition and validation process work products with status `not approved`:
  - Ensure that all product definition and validation process work products are complete and consistent.
  - Identify missing traceability, inconsistencies, or deviations from the defined product definition and validation process.
  - Verify that documentation is DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall update the status of compliant reviewed product definition and validation process work products to `approved`; work products that fail the review shall remain `not approved`.
- Shall clearly state whether the product definition and validation process work products are approved and the process can be closed, or whether further work is needed.

## Resources

- [development_methodology_overview.md](../../../doc/development_methodology/development_methodology_overview.md)
- [product_definition_and_validation_overview.md](../../../doc/development_methodology/product_definition_and_validation/product_definition_and_validation_overview.md)
