#!/usr/bin/env bash
set -euo pipefail

echo "🌐 OpenCode web GUI"

# Validate required commands.
if ! command -v opencode >/dev/null 2>&1; then
    echo "❌ Error: opencode not found."
    exit 1
fi

HOSTNAME="0.0.0.0"
PORT="4096"

exec opencode web --hostname "${HOSTNAME}" --port "${PORT}"
