#!/usr/bin/env bash
set -euo pipefail

echo "🧹 Clean"

# Parse command-line arguments.
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [PRESET] [SOURCE_DIR]"
    exit 1
fi

preset="$1"
source_dir="$2"

# Validate required inputs.
if [ ! -d "${source_dir}" ]; then
    echo "❌ Error: source directory not found: ${source_dir}"
    exit 1
fi

# Clean build artifacts.
build_dir="${source_dir}/build/${preset}"
rm -rf "${build_dir}"

# Clean __pycache__ folders recursively.
find "${source_dir}" -type d -name "__pycache__" -exec rm -rf {} +

echo "✅ Clean completed."
