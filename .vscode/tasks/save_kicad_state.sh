#!/usr/bin/env bash
set -e

echo "💾 Save KiCad state"

# Validate required environment variables.
: "${WORKSPACE_FOLDER:?Missing WORKSPACE_FOLDER}"

workspace_root="${WORKSPACE_FOLDER}"

if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

# Validate required commands.
if ! command -v python3 >/dev/null 2>&1; then
    echo "❌ Error: python3 not found."
    exit 1
fi

echo "💾 Saving KiCad state ..."
python3 "${workspace_root}/tools/kicad_state/save_kicad_state.py"
