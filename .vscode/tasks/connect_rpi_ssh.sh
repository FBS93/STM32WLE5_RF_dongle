#!/usr/bin/env bash
set -euo pipefail

echo "🔗 Connect Raspberry Pi (SSH)"

# Validate required environment variables.
: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"

# Validate required commands.
if ! command -v ssh >/dev/null 2>&1; then
    echo "❌ Error: ssh not found."
    exit 1
fi

echo "Connecting to ${RPI_USER}@${RPI_HOST}..."

exec ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}"
