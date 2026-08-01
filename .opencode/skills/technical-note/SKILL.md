---
name: technical-note
description: >
  Create technical notes for relevant embedded engineering findings.
  Trigger: When a relevant technical finding discovered during a specific task is worth preserving for future work.
---

## When to Use

- When a relevant technical finding should be preserved for future work.
- When a reusable concept, pattern, or implementation note may help future engineering work.

## Critical Patterns

- Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current task.
- Shall create technical notes only for relevant findings that provide future value.
- Shall place each technical note under `doc/technical_notes/<domain>/`, where `<domain>` shall match the engineering domain abbreviation that best fits the primary scope of the note, as defined by the development methodology resources.
- Shall write the technical note in Markdown and keep it self-contained, clear and technical.
- Shall use a concise, searchable file name in lower_snake_case, avoiding generic names such as `note.md` or `misc.md`.
- Shall define the first top-level chapter using the same name as the file name in sentence case. All remaining chapters shall be nested under this first chapter. Nested chapters are free-form according to the needs of the technical note.
- Shall describe verified facts, conclusions, constraints and recommendations, avoiding speculation.
- Shall reference relevant local project documentation when this improves traceability.
- Shall verify that documentation is DRY (Don't Repeat Yourself) by adding references instead of duplicating content.
- Shall avoid repeating concepts already covered by another technical note. If the new finding extends, clarifies, or complements an existing note, it shall update or expand the existing technical note instead of creating a duplicate.

## Resources

- [Development methodology overview](../../../doc/development_methodology/development_methodology_overview.md)
- [Technical notes](../../../doc/technical_notes/)
