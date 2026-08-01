#!/usr/bin/env bash
set -e

echo "⚡ Open KiCad"

# Validate required commands.
if ! command -v kicad >/dev/null 2>&1; then
    echo "❌ Error: kicad not found."
    exit 1
fi

exec kicad
