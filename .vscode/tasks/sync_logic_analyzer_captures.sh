#!/usr/bin/env bash
set -euo pipefail

echo "📥 Sync Logic Analyzer Captures"

# Validate required environment variables.
: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"
: "${LOGIC_ANALYZER_ARTIFACTS_DIR:?Missing LOGIC_ANALYZER_ARTIFACTS_DIR}"

# Parse command-line arguments.
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <workspace_folder>"
    exit 1
fi

workspace_folder="$1"

# Validate required inputs.
if [ ! -d "${workspace_folder}" ]; then
    echo "❌ Error: workspace folder not found: ${workspace_folder}"
    exit 1
fi

if [ ! -d "${LOGIC_ANALYZER_ARTIFACTS_DIR}" ]; then
    echo "❌ Error: logic analyzer artifacts directory not found: ${LOGIC_ANALYZER_ARTIFACTS_DIR}"
    exit 1
fi

# Validate required commands.
if ! command -v ssh >/dev/null 2>&1; then
    echo "❌ Error: ssh not found."
    exit 1
fi

if ! command -v tar >/dev/null 2>&1; then
    echo "❌ Error: tar not found."
    exit 1
fi

artifacts_relative_path="${LOGIC_ANALYZER_ARTIFACTS_DIR#${workspace_folder}/}"

if [ "${artifacts_relative_path}" = "${LOGIC_ANALYZER_ARTIFACTS_DIR}" ]; then
    echo "❌ Error: LOGIC_ANALYZER_ARTIFACTS_DIR must be located inside the workspace folder."
    exit 1
fi

if [ -z "$(ls -A "${LOGIC_ANALYZER_ARTIFACTS_DIR}" 2>/dev/null)" ]; then
    echo "No logic analyzer captures found under ${LOGIC_ANALYZER_ARTIFACTS_DIR}"
    exit 0
fi

ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}" \
    "rm -rf \"/tmp/${artifacts_relative_path}\" && mkdir -p \"/tmp/${artifacts_relative_path%/*}\""

tar -C "${workspace_folder}" -cf - "${artifacts_relative_path}" | \
    ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}" \
    "tar -xf - -C /tmp"

echo "✅ Synced logic analyzer artifacts to ${RPI_USER}@${RPI_HOST}:/tmp/${artifacts_relative_path}"
