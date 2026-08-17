# pdf_to_markdown overview

`pdf_to_markdown.py` converts one local PDF into one explicitly selected Markdown file using Docling. Pictures are embedded in the Markdown as data URIs, so no sidecar asset directory is created.

The generated file begins with a provenance comment used for integrity-checked caching. A matching output is reused without running Docling; a non-matching output is preserved unless replacement is requested with `--force`. Output is published atomically, so a failed conversion does not leave a partial result or replace a previous output.

# Glossary

| Term | Definition |
|---|---|
| Cache key | SHA-256 digest derived from the input PDF, tool and Docling versions, and conversion options. |
| Provenance comment | HTML comment containing the cache key and input and output integrity metadata. |

# Usage example

- `--input <path>`: Existing local `.pdf` file containing a PDF header.
- `--output <path>`: Destination `.md` file. Its parent directory is created when needed; symbolic-link outputs and input/output aliasing are rejected.
- `--force`: Atomically replace an existing output that does not match the requested conversion.

Convert a PDF to an explicitly selected Markdown path:

```bash
python tools/pdf_to_markdown/pdf_to_markdown.py \
  --input path/to/source.pdf \
  --output path/to/result.md
```

On success, the command reports whether Markdown was written or a cached output was reused. Errors are written to standard error and return a non-zero status.
