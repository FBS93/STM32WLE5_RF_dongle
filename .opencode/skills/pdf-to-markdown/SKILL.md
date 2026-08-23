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
- Shall keep every shell call short and shall never poll automatically or wait for job completion.
- Shall run the [PDF-to-Markdown tool](../../../tools/pdf_to_markdown/pdf_to_markdown.md) only through the [`async_job start` command](../../../tools/async_job/async_job.md), never directly.
- Shall query or cancel a job only when explicitly requested.

## Workflow

1. For a conversion, launch from the workspace root with the `async_job start` command:

   ```bash
   python tools/async_job/async_job.py start -- \
     python tools/pdf_to_markdown/pdf_to_markdown.py \
     --input path/to/source.pdf --output path/to/result.md
   ```

   Preserve and report the job identifier `JOB_ID` and end immediately.
2. For a requested query, execute the `async_job list` command once when no job identifier `JOB_ID` is available, noting that it contains only jobs with state `queued` or `running`. If one job cannot be identified, report the candidates and end. Once a job is identified, execute the `async_job status` command and the `async_job log` command exactly once:
   - If the job state reported by the `async_job status` command is `queued` or `running`, report its state and relevant information obtained from the `async_job log` command, then end immediately.
   - If the job state reported by the `async_job status` command is `failed`, `interrupted` or `cancelled`, do not consume the Markdown; report the relevant error and diagnostics obtained from the `async_job log` command.
   - If the job state reported by the `async_job status` command is `succeeded`, require exit code `0` and report relevant completion information obtained from the `async_job log` command.
3. For a requested cancellation, execute the `async_job cancel` command for job identifier `JOB_ID` once, report the result and end.

## Resources

- [async_job tool](../../../tools/async_job/async_job.md)
- [PDF-to-Markdown tool](../../../tools/pdf_to_markdown/pdf_to_markdown.md)
