#!/usr/bin/env bash
set -euo pipefail

echo "🔄 Sync third-party repositories"

# Parse command-line arguments.
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT]"
    exit 1
fi

workspace_root="$1"

# Validate required inputs.
if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

# Validate required commands.
if ! command -v python3 >/dev/null 2>&1; then
    echo "❌ Error: python3 not found."
    exit 1
fi

cd "${workspace_root}"

echo "🔄 Synchronizing third-party repositories ..."
python3 "tools/third_party_sync/third_party_sync.py" --workspace "."
echo "✅ Third-party repositories synchronization completed."
