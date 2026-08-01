# Automatic documentation generation

## Overview

This document describes normal static repository documentation generation with Docgen.

See the [Docgen tool](../../tools/docgen/docgen.md) for its capabilities and details.

## Architecture

- The `📄 Generate documentation` task button generates documentation with the current status of the workspace.

VS Code `📄 Generate documentation` task button --> Docgen --> Documentation output

## Documentation generation setup

The `📄 Generate documentation` task button runs [run_docgen.sh](../../.vscode/tasks/run_docgen.sh). This script:

- Generates documentation for the workspace root.
- Uses the workspace folder basename as the project title.
- Writes the generated documentation to `DOCGEN_OUTPUT`.

## Dependencies

### Devcontainer

Make sure that the environment variables in [devcontainer.json](../../.devcontainer/devcontainer.json) are configured for Docgen:

- `DOCGEN_OUTPUT` defines the directory where Docgen generates documentation.
