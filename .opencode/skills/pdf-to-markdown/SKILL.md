---
name: pdf-to-markdown
description: >
  Convert PDF files to Markdown format.
  Trigger: When explicit PDF-to-Markdown conversion is required or PDF content needs to be processed.
---

## When to Use

- When an explicit PDF-to-Markdown conversion is requested.
- When content from a PDF file needs to be processed.

## Critical Patterns

- - Shall strictly follow the provided resources and recursively review all referenced documents required to understand the current task.
- Shall keep every shell call short and shall never poll more than once or wait for job completion.
- Shall run the [PDF-to-Markdown tool](../../../tools/pdf_to_markdown/pdf_to_markdown.md) only through [`async_job start`](../../../tools/async_job/async_job.md) command, never directly.
- Shall query or cancel a job only when explicitly requested.

## Workflow

1. For a conversion, launch from the workspace root with `async_job start`:

   ```bash
   python tools/async_job/async_job.py start -- \
     python tools/pdf_to_markdown/pdf_to_markdown.py \
     --input path/to/source.pdf --output path/to/result.md
   ```

   Report the job ID, selected output path and `pending` status, then end immediately without querying status or reading the output.
2. For a requested query, use `list` once when no job ID is available, noting that it contains only active jobs. If one job cannot be identified, report the candidates and end. Once a job is identified, call `status` exactly once:
   - If active, report its state and end immediately.
   - If `failed`, `interrupted` or `cancelled`, do not consume the Markdown; use `log` only when it adds useful diagnostic information.
   - If `succeeded`, require exit code 0. Before consuming required Markdown, verify that the selected output is a regular file.
3. For a requested cancellation, call `async_job cancel JOB_ID`, report the result and end without polling.

## Resources

- [async_job tool](../../../tools/async_job/async_job.md)
- [PDF-to-Markdown tool](../../../tools/pdf_to_markdown/pdf_to_markdown.md)
