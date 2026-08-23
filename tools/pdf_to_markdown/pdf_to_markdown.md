# pdf_to_markdown overview

`pdf_to_markdown.py` converts one local PDF into one explicitly selected Markdown file using Docling. The PDF is processed sequentially in page segments; each segment runs in a fresh child process and the parent concatenates its temporary Markdown output. Pictures are embedded in the Markdown as data URIs, so no sidecar asset directory is created.

The base segment size defaults to ten pages and can be combined with forced page ranges. A fresh process prevents memory from accumulating across segments, but a single segment can still exhaust available memory. Fresh processes reload the converter and models, so smaller segments improve isolation at the cost of initialization time. 

The generated file begins with a provenance comment used for integrity-checked caching. A matching output is reused without running Docling; a non-matching output is preserved unless replacement is requested with `--force`. Output is published atomically, so a failed segment does not leave a partial result or replace a previous output.

# Glossary

| Term | Definition |
|---|---|
| Cache key | SHA-256 digest derived from the input PDF, tool and Docling versions, and conversion options. |
| Provenance comment | HTML comment containing the cache key and input and output integrity metadata. |

# Usage example

- `--input <path>`: Existing local `.pdf` file containing a PDF header.
- `--output <path>`: Destination `.md` file. Its parent directory is created when needed; symbolic-link outputs and input/output aliasing are rejected.
- `--force`: Atomically replace an existing output that does not match the requested conversion.
- `--verbose`: Show Docling logging and stream output during conversion.
- `--pages-per-segment <N>`: Positive base segment size. Defaults to `10`.
- `--page-ranges <ranges>`: Optional comma-separated inclusive, one-based forced ranges, for example `50-53,600-601`. Each range remains one segment and may be combined with `--pages-per-segment`.

Forced ranges must be ordered by start page, non-overlapping and within the PDF. Gaps use the base segmentation; adjacent forced ranges remain separate. A page range containing one page is written as `x-x`. Malformed, reversed, duplicated, overlapping or out-of-bounds ranges fail before conversion.

Convert a PDF to an explicitly selected Markdown path:

```bash
python tools/pdf_to_markdown/pdf_to_markdown.py \
  --input path/to/source.pdf \
  --output path/to/result.md \
  --pages-per-segment 8 \
  --page-ranges 50-53,600-601
```

On success, the command reports whether Markdown was written or a cached output was reused. Errors are written to standard error and return a non-zero status.
