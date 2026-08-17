# Test PDF to Markdown test overview

Black-box tests for the PDF-to-Markdown command-line interface.

# Use case

Temporary PDF inputs, Markdown destinations, and a deterministic Docling substitute exercise conversion, cache reuse, replacement, validation, and conversion failures through separate command processes.

# Verification scope

The tests verify user-visible exit status, output and error messages, exact destination publication, cache reuse, explicit replacement, invalid-path rejection, staging cleanup, and preservation of previous output on failure.
