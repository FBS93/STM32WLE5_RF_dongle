# Test PDF to Markdown test overview

Black-box tests for the PDF-to-Markdown command-line interface.

# Use case

The test scenarios cover:

- Temporary PDF inputs, Markdown destinations and a deterministic Docling substitute are exercised through separate command processes.
- Segmentation and conversion cover the default ten-page segmentation, explicit segment-size overrides, forced ranges, exact page coverage and progress.
- Output handling covers cache reuse, replacement, validation, conversion failures and atomic publication.

# Verification scope

The tests verify:

- User-visible behavior covers exit status, output and error messages, plus TTY progress feedback including total elapsed refreshes and non-TTY progress feedback.
- Conversion behavior covers ordered non-overlapping Docling page ranges, exact destination publication, cache differentiation, explicit replacement, and invalid-path and segmentation rejection.
- Failure handling covers captured Docling diagnostics, staging cleanup and preservation of previous output on failure.
