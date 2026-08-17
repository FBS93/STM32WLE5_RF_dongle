#!/usr/bin/env bash
set -euo pipefail

echo "📄 Generate documentation"

# Parse command-line arguments.
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT] [TITLE]"
    exit 1
fi

workspace_root="${1%/}"
title="$2"

# Validate required inputs.
if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

# Validate required environment variables.
if [ -z "${DOCGEN_OUTPUT-}" ]; then
    echo "❌ Error: DOCGEN_OUTPUT must define the documentation output directory."
    exit 1
fi

# Validate required commands.
if ! command -v python3 >/dev/null 2>&1; then
    echo "❌ Error: python3 not found."
    exit 1
fi

cd "${workspace_root}"

echo "Generating ..."
if python3 "tools/docgen/docgen.py" --root . --output "${DOCGEN_OUTPUT}" \
    --title "${title}" --exclude third_party; then
    echo "✅ Generated: ${DOCGEN_OUTPUT}/index.html"
else
    status=$?
    echo "❌ Documentation generation failed with status ${status}." >&2
    exit "${status}"
fi
